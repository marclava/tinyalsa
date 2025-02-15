/**
 * @file pcm-readi-mmap.c
 * @brief recording ALSA PCM hardware device in MMAP mode (example)
 * @author Marc Lavallée
 * @version 0.1
 * @date 2025-02-15
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>

#include <tinyalsa/asoundlib.h>


struct pcm* pcm_cap = NULL;
FILE* pcmfile = NULL;
static bool running = true;

// to stop recording using signals (ex. Ctrl-C)
static void sighandler(int sig)
{
    fprintf(stderr, "\n%s\n", strsignal(sig));
    running = false;
}


// use_pcm_areas callback for pcm_readi (with pcm device in MMAP mode)
void read_pcm_areas(struct pcm* pcm, char* areas,
    unsigned int frame_offset, unsigned int frames)
{   
    if (! frames) return;

    // bytes per captured frame
    unsigned short bpcf = pcm_frames_to_bytes(pcm, 1); 

    // starting address to read from MMAP areas
    areas += bpcf * frame_offset; 

    // write raw pcm data, else stop recording on error
    if (! fwrite(areas, bpcf, frames, pcmfile)) {
        fprintf(stderr, "error writing to raw PCM file.\n");
        running = false;
    }
}

int main()
{
    // capture device must be configured
    // lowering period-size may help lowering latency
    const struct pcm_config cap_config = {
        .channels = 2,
        .rate = 48000,
        .period_size = 1024,
        .period_count = 2,
        .format = PCM_FORMAT_S32_LE
    };

    // set the ALSA device indexes
    // for a list, inspect /proc/asound/cards
    unsigned int card = 2;
    unsigned int device = 0;

    // open capture device
    // MMAP mode required with "use_pcm_areas" callback for pcm_readi
    unsigned int flags = PCM_IN|PCM_MMAP;
    pcm_cap = pcm_open(card, device, flags, &cap_config);
    if (!pcm_cap || !pcm_is_ready(pcm_cap)) {
        fprintf(stderr, "Unable to open PCM capture device (%s)\n",
            pcm_get_error(pcm_cap));
        return EXIT_FAILURE;
    }

    // open capture file
    pcmfile = fopen("out.raw", "wb");
    if (!pcmfile) {
        fprintf(stderr, "Failed to create output file.\n");
        exit(EXIT_FAILURE);
    }

    // init signal handler
    signal(SIGINT,  sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGHUP, sighandler);

    unsigned int nb_frames = pcm_get_buffer_size(pcm_cap);

    do {
        int frames_read = pcm_readi(pcm_cap, NULL, nb_frames, read_pcm_areas);
        // fprintf(stderr,"%d frames\n", nb_frames);
        if (!running) break;

        if ((unsigned int)frames_read < nb_frames) {
            if (frames_read < 0) {
                fprintf(stderr,"Error capturing frames\n");
            }
            else {
                fprintf(stderr, "Missing nb_frames; %d captured, %d expected.\n",
                    frames_read, nb_frames);
            }
            running = false;
        }

    } while (running);

    // close and free
    if (pcm_cap)
        pcm_close(pcm_cap);
    if (pcmfile) {
        fflush(pcmfile);
        fclose(pcmfile);
    }

    // done!
    return EXIT_SUCCESS;
}

// EOF
