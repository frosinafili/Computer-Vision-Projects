#include <stdio.h>
#include <stdlib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

/*I started from the demuxing_decoding.c example, removed audio handling and raw video output, 
simplified the decode loop, and modified it to stop at a specific frame and save it as 
PPM and PGM.*/

void save_ppm(uint8_t *rgb, int width, int height, const char *filename)
{
    FILE *f = fopen(filename, "wb");
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    for (int y = 0; y < height; y++)
        fwrite(rgb + y * width * 3, 1, width * 3, f);
    fclose(f);
}

void save_pgm_formula(uint8_t *rgb, int width, int height,
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

        int gray = rcoef*r + gcoef*g + bcoef*b;
        if (gray > 255) gray = 255;
        if (gray < 0) gray = 0;

        uint8_t val = (uint8_t)gray;
        fwrite(&val, 1, 1, f);
    }

    fclose(f);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <video> <frame_number>\n", argv[0]);
        return -1;
    }

    char *filename = argv[1];
    int target_frame = atoi(argv[2]);

    AVFormatContext *format_ctx = NULL;
    avformat_open_input(&format_ctx, filename, NULL, NULL);
    avformat_find_stream_info(format_ctx, NULL);

    av_dump_format(format_ctx, 0, filename, 0);  // Print metadata

    int video_stream_index = -1;

    for (int i = 0; i < format_ctx->nb_streams; i++)
    {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            break;
        }
    }

    printf("video stream index: %d\n", video_stream_index);

    AVCodecParameters *codecpar =
        format_ctx->streams[video_stream_index]->codecpar;

    const AVCodec *codec =
        avcodec_find_decoder(codecpar->codec_id);

    AVCodecContext *codec_ctx =
        avcodec_alloc_context3(codec);

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

    uint8_t *rgb_buffer =
        malloc(codec_ctx->width * codec_ctx->height * 3);

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

                    save_ppm(rgb_buffer,
                             codec_ctx->width,
                             codec_ctx->height,
                             "image0.ppm");

                    save_pgm_formula(rgb_buffer,
                                     codec_ctx->width,
                                     codec_ctx->height,
                                     1.0/3, 1.0/3, 1.0/3,
                                     "image1.pgm");

                    save_pgm_formula(rgb_buffer,
                                     codec_ctx->width,
                                     codec_ctx->height,
                                     0.6, 0.2, 0.2,
                                     "image2.pgm");

                    save_pgm_formula(rgb_buffer,
                                     codec_ctx->width,
                                     codec_ctx->height,
                                     0.2, 0.6, 0.2,
                                     "image3.pgm");

                    save_pgm_formula(rgb_buffer,
                                     codec_ctx->width,
                                     codec_ctx->height,
                                     0.2, 0.2, 0.6,
                                     "image4.pgm");

                    printf("written frame %d into image0.ppm\n", target_frame);
                    printf("written frame %d into image1.pgm with (r+g+b)/3\n", target_frame);
                    printf("written frame %d into image2.pgm with 0.6r + 0.2g + 0.2b\n", target_frame);
                    printf("written frame %d into image3.pgm with 0.2r + 0.6g + 0.2b\n", target_frame);
                    printf("written frame %d into image4.pgm with 0.2r + 0.2g + 0.6b\n", target_frame);

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