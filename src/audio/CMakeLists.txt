# Audio
#######

target_sources(${PROJECT_NAME} PRIVATE
	decoder.cpp
	decoder.h
	decoder_base.cpp
	decoder_base.h
	decoder_midi.cpp
	decoder_midi.h
	generic.cpp
	generic.h
	generic_midiout.cpp
	generic_midiout.h
	interface.cpp
	interface.h
	midi.cpp
	midi.h
	midisequencer.cpp
	midisequencer.h
	secache.cpp
	secache.h)

if(NOT PLAYER_HAS_AUDIO)
	# No need to link in external libraries
	return()
endif()

if(PLAYER_HAVE_LIBSPEEXDSP OR PLAYER_HAVE_LIBSAMPLERATE)
	target_sources(${PROJECT_NAME} PRIVATE resampler.cpp resampler.h)
endif()

if(PLAYER_ENABLE_FMMIDI)
	target_sources(${PROJECT_NAME} PRIVATE
		decoder_fmmidi.cpp
		decoder_fmmidi.h
		midiprogram.h
		midisynth.cpp
		midisynth.h)
endif()

if(PLAYER_ENABLE_DRWAV)
	target_compile_definitions(${PROJECT_NAME} PUBLIC WANT_DRWAV=1)
	target_sources(${PROJECT_NAME} PRIVATE
		decoder_drwav.cpp
		decoder_drwav.h
		${PROJECT_SOURCE_DIR}/src/external/dr_wav.h)
endif()

if(PLAYER_HAVE_FLUIDLITE OR PLAYER_HAVE_FLUIDSYNTH)
	target_sources(${PROJECT_NAME} PRIVATE decoder_fluidsynth.cpp decoder_fluidsynth.h)
endif()

if(PLAYER_HAVE_LIBSNDFILE)
	target_sources(${PROJECT_NAME} PRIVATE decoder_libsndfile.cpp decoder_libsndfile.h)
endif()

if(PLAYER_HAVE_LIBMPG123)
	target_sources(${PROJECT_NAME} PRIVATE decoder_mpg123.cpp decoder_mpg123.h)
endif()

if(PLAYER_HAVE_OGGVORBIS OR PLAYER_HAVE_TREMOR)
	target_sources(${PROJECT_NAME} PRIVATE decoder_oggvorbis.cpp decoder_oggvorbis.h)
endif()

if(PLAYER_HAVE_OPUS)
	target_sources(${PROJECT_NAME} PRIVATE decoder_opus.cpp decoder_opus.h)
endif()

if(PLAYER_HAVE_LIBWILDMIDI)
	target_sources(${PROJECT_NAME} PRIVATE decoder_wildmidi.cpp decoder_wildmidi.h)
endif()

if(PLAYER_HAVE_LIBXMP)
	target_sources(${PROJECT_NAME} PRIVATE decoder_xmp.cpp decoder_xmp.h)
endif()
