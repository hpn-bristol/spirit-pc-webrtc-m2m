#pragma once

#include <cstdint>

#ifndef DLLExport
#  if defined(_WIN32) || defined(_WIN64)
#    define DLLExport __declspec(dllexport)
#  else
#    define DLLExport __attribute__((visibility("default")))
#  endif
#endif

// All exported functions should be declared here

#ifdef __cplusplus
extern "C" {
#endif

  // --------- Aditions ---------
  
  typedef struct EncodingQueue   EncodingQueue;
  typedef struct DracoMDCDecoder DracoMDCDecoder;
  // typedef struct DracoMDCEncoder DracoMDCEncoder;
  typedef struct Description     Description;
  typedef struct PointCloud      PointCloud;
  
  typedef void (*DescriptionDoneCallback)(
  	Description* dsc,
  	char*        raw_data_ptr,
  	uint32_t     n_points_in_total,
  	uint32_t     dsc_size,
  	uint32_t     capturer_id,
  	uint32_t     frame_nr,
  	uint32_t     dsc_nr,
  	uint64_t     timestamp);

  typedef void (*FreePointCloudCallback)(PointCloud* pc);

	DLLExport void register_description_done_callback(EncodingQueue* enc_queue, DescriptionDoneCallback cb);
	DLLExport void register_free_pc_callback(EncodingQueue* enc_queue, FreePointCloudCallback cb);
	// -----

	DLLExport int initialize();
	DLLExport void set_logging(char* log_directory, int _log_level);
	DLLExport EncodingQueue* create_encoding_queue(unsigned int max_queue);
	DLLExport uint32_t encode_pc(EncodingQueue* enc, PointCloud* pc);
	DLLExport void clean_up();
	// DLLExport uint32_t get_encoded_size(DracoMDCEncoder* enc);
	// DLLExport char* get_raw_data(DracoMDCEncoder* enc);
	DLLExport DracoMDCDecoder* decode_pc(char* data, uint32_t size);
	DLLExport uint32_t get_n_points(DracoMDCDecoder* dec); 
	DLLExport float* get_point_array(DracoMDCDecoder* dec);
	DLLExport uint8_t* get_color_array(DracoMDCDecoder* dec);
	DLLExport void free_encoding_queue(EncodingQueue* enc);
	// DLLExport void free_encoder(DracoMDCEncoder* enc);
	DLLExport void free_decoder(DracoMDCDecoder* dec);
	DLLExport void free_description(Description* dsc);


#ifdef __cplusplus
}
#endif
