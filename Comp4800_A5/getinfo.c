#include <libavformat/avformat.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    AVFormatContext *fmt_ctx = NULL;
    int i;

    if (argc != 3) {
        printf("usage: argv[0] filename type(0:dump,1:selected)\n");
        return 1;
    }

    if (avformat_open_input(&fmt_ctx, argv[1], NULL, NULL) < 0)
        return 1;

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
        return 1;

    if (atoi(argv[2]) == 0) {
        av_dump_format(fmt_ctx, 0, argv[1], 0);
    } else {
        printf("number of streams: %d\n", fmt_ctx->nb_streams);

        int video_index = -1;
        for (i = 0; i < fmt_ctx->nb_streams; i++)
            if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                video_index = i;

        if (video_index == -1)
            return 1;

        double fps = av_q2d(fmt_ctx->streams[video_index]->avg_frame_rate);
        printf("frame rate: %.1f\n", fps);

        int total = fmt_ctx->duration / AV_TIME_BASE;
        printf("duration: %d min %d sec\n", total/60, total%60);
    }
    avformat_close_input(&fmt_ctx);
    return 0;
}