#include <stdio.h>
#include <stdlib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

void save_ppm(uint8_t *rgb, int width, int height, const char *filename)
{
    FILE *f = fopen(filename, "wb");
    fprintf(f, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; y++)
        fwrite(rgb + y * width * 3, 1, width * 3, f);

    fclose(f);
}

void save_pgm(uint8_t *rgb, int width, int height,
              double rcoef, double gcoef, double bcoef,
              const char *filename)
{
    FILE *f = fopen(filename, "wb");
    fprintf(f, "P5\n%d %d\n255\n", width, height);

    for (int i = 0; i < width * height; i++)
    {
        int r = rgb[i * 3];
        int g = rgb[i * 3 + 1];
        int b = rgb[i * 3 + 2];

        uint8_t gray = (uint8_t)(rcoef*r + gcoef*g + bcoef*b);
        fwrite(&gray, 1, 1, f);
    }

    fclose(f);
}

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        printf("Usage: %s <video> <frame_number> <Rcoef> <Gcoef> <Bcoef>\n", argv[0]);
        return -1;
    }

    char *filename = argv[1];
    int target_frame = atoi(argv[2]);

    double rcoef = atof(argv[3]);
    double gcoef = atof(argv[4]);
    double bcoef = atof(argv[5]);

    AVFormatContext *format_ctx = NULL;
    avformat_open_input(&format_ctx, filename, NULL, NULL);
    avformat_find_stream_info(format_ctx, NULL);

    int video_stream_index = -1;

    for (int i = 0; i < format_ctx->nb_streams; i++)
    {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1)
    {
        printf("No video stream found\n");
        return -1;
    }

    AVCodecParameters *codecpar = format_ctx->streams[video_stream_index]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codecpar);
    avcodec_open2(codec_ctx, codec, NULL);

    AVPacket packet;
    AVFrame *frame = av_frame_alloc();

    struct SwsContext *sws_ctx =
        sws_getContext(codec_ctx->width,
                       codec_ctx->height,
                       codec_ctx->pix_fmt,
                       codec_ctx->width,
                       codec_ctx->height,
                       AV_PIX_FMT_RGB24,
                       SWS_BILINEAR,
                       NULL, NULL, NULL);

    uint8_t *rgb_buffer = malloc(codec_ctx->width * codec_ctx->height * 3);
    uint8_t *rgb_data[1] = { rgb_buffer };
    int rgb_linesize[1] = { 3 * codec_ctx->width };

    int frame_count = 0;

    while (av_read_frame(format_ctx, &packet) >= 0)
    {
        if (packet.stream_index == video_stream_index)
        {
            avcodec_send_packet(codec_ctx, &packet);

            while (avcodec_receive_frame(codec_ctx, frame) == 0)
            {
                if (frame_count == target_frame)
                {
                    sws_scale(sws_ctx,
                              (const uint8_t * const*)frame->data,
                              frame->linesize,
                              0,
                              codec_ctx->height,
                              rgb_data,
                              rgb_linesize);

                    save_ppm(rgb_buffer, codec_ctx->width, codec_ctx->height, "frame.ppm");

                    save_pgm(rgb_buffer,
                             codec_ctx->width,
                             codec_ctx->height,
                             rcoef, gcoef, bcoef,
                             "frame.pgm");

                    printf("Frame %d saved as frame.ppm and frame.pgm\n", target_frame);
                    goto end;
                }

                frame_count++;
            }
        }

        av_packet_unref(&packet);
    }

end:

    free(rgb_buffer);
    sws_freeContext(sws_ctx);
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);

    return 0;
}