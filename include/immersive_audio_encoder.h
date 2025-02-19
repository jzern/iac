#ifndef IMMERSIVE_AUDIO_ENCODER_H
#define IMMERSIVE_AUDIO_ENCODER_H
#include <stdint.h>

#include "immersive_audio_defines.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct IAEncoder IAEncoder;

#ifndef DEMIXING_MATRIX_SIZE_MAX
#define DEMIXING_MATRIX_SIZE_MAX (18 * 18 * 2)
#endif
typedef struct {
  int output_channel_count;
  int substream_count;
  int coupled_substream_count;
  int channel_mapping[12]; // todo 12?
  int demixing_matrix[DEMIXING_MATRIX_SIZE_MAX]; // todo DEMIXING_MATRIX_SIZE_MAX?
}AMBISONICS_LAYER_CONFIG;

typedef struct {
  int loudspeaker_layout;
  int output_gain_is_present_flag;
  int recon_gain_is_present_flag;
  int substream_count;
  int coupled_substream_count;
  int loudness;
  int output_gain_flags;
  int output_gain;
}CHANNEL_AUDIO_LAYER_CONFIG;

typedef struct {
  int version;
  int ambisonics_mode;
  int channel_audio_layer;

  AMBISONICS_LAYER_CONFIG ambisonics_layer_config;
  CHANNEL_AUDIO_LAYER_CONFIG channel_audio_layer_config[IA_CHANNEL_LAYOUT_COUNT];
}IA_STATIC_METADATA;




/**
* @brief     Create immersive audio encoder handler
* @param     [in] Fs : sample rate.
* @param     [in]channel_layout_in : the input of channel layout, ex: 7.1.4
* @param     [in]channel_layout_cb : the scalable channel layout combinations, ex: 2ch / 3.1.2ch / 5.1.2ch
* @param     [in]codec_id : the codec id. 1:opus, 2:aac
* @param     [in]error : the return error when create ia handle
* @return    return immersive audio encoder handler
* @remark    Adjacent channel layouts of a scalable format (where CLn-1 is the precedent channel layout and CLn is the next one) are
*            only allowed as below, where CLn = S(n).W(n).H(n)
*            S(n-1) ? S(n) and W(n 1) ? W(n) and H(n 1) ? H(n) except "S(n 1) = S(n) and W(n 1) = W(n) and H(n 1) = H(n)"
*            NOTE: S(Surround Channel), W(Subwoofer Channel), H(Height Channel)
*            if [in]channel_layout_cb is set with IA_CHANNEL_LAYOUT_COUNT, then it is non-scalable encoding.
*/
IAEncoder *immersive_audio_encoder_create(int32_t Fs,
  IAChannelLayoutType channel_layout_in,
  const IAChannelLayoutType *channel_layout_cb,//
  int codec_id, //1:opus, 2:aac
  int *error);

/*
Following 3 apis are used to implement Down-Mix Parameter Determination.
*/
/**
* @brief     Prepare the starting of own-Mix Parameter Determination
* @param     [in] st : channel group encoder handle.
* @return    @0: success,@others: fail
*/
int immersive_audio_encoder_dmpd_start(IAEncoder *st);

/**
* @brief     Down-Mix Parameter Determination pre-process.
* @param     [in] st : channel group encoder handle.
* @return    @0: success,@others: fail
*/
int immersive_audio_encoder_dmpd_process(IAEncoder *st, const int16_t *pcm, int frame_size);

/**
* @brief     Stop the Down-Mix Parameter Determination pre-process.
* @param     [in] st : channel group encoder handle.
* @return    @0: success,@others: fail
*/
int immersive_audio_encoder_dmpd_stop(IAEncoder *st);

/*
Following 2 apis are used to calculate loundness and output_gain.
*/

/**
* @brief     loudness &gain process
* @param     [in] st : immersive audio encoder handle.
* @param     [in] pcm : input pcm sample data.
* @param     [in] frame_size : frame size of input pcm.
* @return    @0: success,@others: fail
*/
int immersive_audio_encoder_loudness_gain(IAEncoder *st, const int16_t *pcm, int frame_size);

/**
* @brief     End the loudness &gain process and output the values
* @param     [in] st : immersive audio encoder handle.
* @return    @0: success,@others: fail
*/
int immersive_audio_encoder_loudness_gain_end(IAEncoder *st);

/**
* @brief     Return IA static metadata which is used in mp4 muxer
* @param     [in] st : immersive audio encoder handle.
* @return    return the ia_static_metada sturcture
*/
IA_STATIC_METADATA get_immersive_audio_encoder_ia_static_metadata(IAEncoder *st);


/**
* @brief     Calculate the recon gain.
* @param     [in] st : immersive audio encoder handle.
* @return    @0: success,@others: fail
*/
int immersive_audio_encoder_recon_gain(IAEncoder *st, const int16_t *pcm, int frame_size);

/**
* @brief     Encode a ia ia frame.
* @param     [in] st : immersive audio encoder handle.
* @param     [in] pcm : input pcm sample data.
* @param     [in] frame_size : Number of samples per channel in the input signal.
* @param     [in] data : Output payload.
* @param     [in] max_data_bytes : Size of the allocated memory for the output payload
* @return    The length of the encoded packet (in bytes) on success or a
*            negative error code (see @ref IAErrCode) on failure.
*/
int immersive_audio_encode(IAEncoder *st, const int16_t *pcm, int frame_size, unsigned char* data, int *demix_mode, int32_t max_data_bytes);

/**
* @brief     Perform a CTL function on a  immersive audio encoder.
* @param     [in] st : immersive audio encoder handle.
* @param     [in] request : control request.
* @return    @0: success,@others: fail
*/
int immersive_audio_encoder_ctl(IAEncoder *st, int request, ...);

/**
* @brief     Free the immersive audio encoder.
* @param     [in] st : immersive audio encoder handle.
*/
void immersive_audio_encoder_destroy(IAEncoder *st);

#ifdef __cplusplus
}
#endif

#endif /* IMMERSIVE_AUDIO_ENCODER_H */

