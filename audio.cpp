#include <unistd.h>
#include <stdlib.h>
extern "C" {
#include <soundpipe.h>
#include <sporth.h>
#include "spigot.h"
}
#include "RtAudio.h"

spigot_graphics *global_gfx = NULL;

static int callme( void * outputBuffer, void * inputBuffer, unsigned int numFrames,
            double streamTime, RtAudioStreamStatus status, void * data )
{
    SPFLOAT *output = (SPFLOAT *) outputBuffer;    
    unsigned int i;
    
    plumber_data *pd = (plumber_data *)data;

    for(i = 0; i < numFrames * 2; i+= 2) {
        plumber_compute(pd, PLUMBER_COMPUTE);
        output[i] = sporth_stack_pop_float(&pd->sporth.stack);
        output[i + 1] = output[i];
    }
    return 0;
}

void spigot_start_audio(plumber_data *pd, spigot_state *state, int use_gfx)
{
    RtAudio audio;
    RtAudio::DeviceInfo info; 
    RtAudio::StreamParameters iParams, oParams;
    unsigned int buffer_frames = 1024;
    info = audio.getDeviceInfo(audio.getDefaultOutputDevice());
    iParams.deviceId = audio.getDefaultInputDevice();
    iParams.nChannels = 0;
    iParams.firstChannel = 0;
    oParams.deviceId = audio.getDefaultOutputDevice();
    oParams.nChannels = 2;
    oParams.firstChannel = 0;
    
    RtAudio::StreamOptions options;
    
    audio.openStream( &oParams, NULL, 
            RTAUDIO_FLOAT32, info.preferredSampleRate, 
            &buffer_frames, &callme, pd, &options);
    audio.showWarnings( true );

    global_gfx = spigot_gfx_new(3);
    spigot_set_recompile(global_gfx, &pd->recompile);
    
    if(!spigot_loaded(global_gfx) && spigot_is_it_happening(global_gfx)) {
        plumber_print(pd, "Spigot state not loaded!\n");
    }

    spigot_gfx_set_state(global_gfx, state);
    audio.startStream();
    if(use_gfx) spigot_start_why_dont_you(global_gfx);

    if(spigot_is_it_happening(global_gfx)) {
        spigot_gfx_init(global_gfx);
        spigot_graphics_loop(global_gfx);
    } else {
        fgetc(stdin);
    }

    audio.stopStream();
    spigot_gfx_free(global_gfx);
}

static int rproc_audio(runt_vm *vm, runt_ptr p)
{
    plumber_data *pd;
    int rc;
    runt_stacklet *s;
    int use_gfx;
    spigot_state *state;

    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    pd = (plumber_data *)runt_to_cptr(s->p);
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    state = (spigot_state *)runt_to_cptr(s->p);
    
    rc = runt_ppop(vm, &s);
    RUNT_ERROR_CHECK(rc);
    use_gfx = s->f;

    spigot_start_audio(pd, state, use_gfx);
    return RUNT_OK;
}

static int rproc_gfx_step(runt_vm *vm, runt_ptr p)
{
    if(global_gfx == NULL) return RUNT_OK;
    if(spigot_is_it_happening(global_gfx)) {
        spigot_gfx_step(global_gfx);
    }
    return RUNT_OK;
}

void spigot_audio_runt(runt_vm *vm, runt_ptr p)
{
    spigot_word_define(vm, p, "spigot_audio", 12, rproc_audio);
    spigot_word_define(vm, p, "spigot_gfx_step", 15, rproc_gfx_step);
}
