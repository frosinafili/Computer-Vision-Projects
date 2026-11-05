#include <opencv2/opencv.hpp>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

// ──────────────────────────────────────────────
//  Circular buffer
// ──────────────────────────────────────────────
#define BUFFER_SIZE 10

struct Frame {
    uint8_t* data;
    int width;
    int height;
};

static Frame  circ_buf[BUFFER_SIZE];
static int    head    = 0;
static int    tail    = 0;
static int    count   = 0;

static pthread_mutex_t mtx        = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond_full  = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  cond_empty = PTHREAD_COND_INITIALIZER;

static bool finished = false;
static int  fps      = 25;

static void push_frame(Frame f) {
    pthread_mutex_lock(&mtx);
    while (count >= BUFFER_SIZE)
        pthread_cond_wait(&cond_full, &mtx);
    circ_buf[head] = f;
    head = (head + 1) % BUFFER_SIZE;
    count++;
    pthread_cond_signal(&cond_empty);
    pthread_mutex_unlock(&mtx);
}

static Frame pop_frame() {
    pthread_mutex_lock(&mtx);
    while (count == 0 && !finished)
        pthread_cond_wait(&cond_empty, &mtx);
    Frame f;
    if (count > 0) {
        f = circ_buf[tail];
        tail = (tail + 1) % BUFFER_SIZE;
        count--;
        pthread_cond_signal(&cond_full);
    } else {
        f.data = NULL;
    }
    pthread_mutex_unlock(&mtx);
    return f;
}

// ──────────────────────────────────────────────
//  Decoder thread  (producer)
// ──────────────────────────────────────────────
static void* decoder_thread(void* arg) {
    const char* filename = (const char*)arg;

    AVFormatContext* format_ctx = NULL;
    if (avformat_open_input(&format_ctx, filename, NULL, NULL) < 0) {
        printf("Could not open file: %s\n", filename);
        finished = true;
        pthread_cond_broadcast(&cond_empty);
        return NULL;
    }
    avformat_find_stream_info(format_ctx, NULL);

    int video_stream_index = -1;
    for (int i = 0; i < (int)format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1) {
        printf("No video stream found\n");
        finished = true;
        pthread_cond_broadcast(&cond_empty);
        avformat_close_input(&format_ctx);
        return NULL;
    }

    AVCodecParameters* codecpar  = format_ctx->streams[video_stream_index]->codecpar;
    const AVCodec*     codec     = avcodec_find_decoder(codecpar->codec_id);
    AVCodecContext*    codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codecpar);
    avcodec_open2(codec_ctx, codec, NULL);

    AVPacket* packet = av_packet_alloc();
    AVFrame*  frame  = av_frame_alloc();

    SwsContext* sws_ctx = sws_getContext(
        codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
        codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, NULL, NULL, NULL
    );

    while (av_read_frame(format_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            avcodec_send_packet(codec_ctx, packet);
            while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                int buf_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24,
                                   codec_ctx->width, codec_ctx->height, 1);
                uint8_t* buffer_rgb = (uint8_t*)malloc(buf_size);
                uint8_t* dest[1]     = { buffer_rgb };
                int      linesize[1] = { 3 * codec_ctx->width };
                sws_scale(sws_ctx, frame->data, frame->linesize,
                          0, codec_ctx->height, dest, linesize);
                Frame f;
                f.data   = buffer_rgb;
                f.width  = codec_ctx->width;
                f.height = codec_ctx->height;
                push_frame(f);
            }
        }
        av_packet_unref(packet);
    }

    finished = true;
    pthread_cond_broadcast(&cond_empty);

    av_packet_free(&packet);
    av_frame_free(&frame);
    sws_freeContext(sws_ctx);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);

    return NULL;
}

// ──────────────────────────────────────────────
//  Display thread  (consumer)
// ──────────────────────────────────────────────
static void* display_thread(void* arg) {
    int delay = 1000 / fps;
    while (true) {
        Frame f = pop_frame();
        if (f.data == NULL)
            break;
        cv::Mat img(f.height, f.width, CV_8UC3, f.data);
        cv::imshow("Video", img);
        cv::waitKey(delay);
        free(f.data);
    }
    return NULL;
}

// ──────────────────────────────────────────────
//  main
// ──────────────────────────────────────────────
int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./a7 <video> <fps>\n");
        return -1;
    }
    fps = atoi(argv[2]);
    pthread_t t1, t2;
    pthread_create(&t1, NULL, decoder_thread, argv[1]);
    pthread_create(&t2, NULL, display_thread, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}