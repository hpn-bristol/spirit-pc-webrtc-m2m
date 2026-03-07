#include "plugin.h"             // public C ABI we're testing
#include <cstdint>              // fixed-width types
#include <vector>

#include <draco/point_cloud/point_cloud.h>
#include <draco/point_cloud/point_cloud_builder.h>
#include <draco/compression/encode.h>

static void make_tiny_draco(std::vector<char>& out) {
  // Build a 4-point cloud with POSITION+COLOR
  draco::PointCloudBuilder b;
  const int N = 4;
  b.Start(N);
  const int att_pos = b.AddAttribute(draco::GeometryAttribute::POSITION, 3, draco::DT_FLOAT32);
  const int att_col = b.AddAttribute(draco::GeometryAttribute::COLOR,    3, draco::DT_UINT8);

  // A tiny square with a simple colour ramp
  struct P { float x,y,z; };
  const P pts[N] = {{0,0,0},{1,0,0},{1,1,0},{0,1,0}};
  const uint8_t cols[N][3] = {{10,20,30},{40,50,60},{70,80,90},{100,110,120}};

  for (int i = 0; i < N; ++i) {
    b.SetAttributeValueForPoint(att_pos, draco::PointIndex(i), &pts[i]);
    b.SetAttributeValueForPoint(att_col, draco::PointIndex(i), cols[i]);
  }

  std::unique_ptr<draco::PointCloud> pc = b.Finalize(false);

  draco::Encoder enc;
  enc.SetEncodingMethod(draco::POINT_CLOUD_KD_TREE_ENCODING);
  enc.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 11);

  draco::EncoderBuffer buf;
  auto ok = enc.EncodePointCloudToBuffer(*pc, &buf).ok();
  if (!ok) return;

  out.assign(buf.data(), buf.data() + buf.size());
}

int main() {
  // 1) Generate valid Draco bytes (no files involved)
  std::vector<char> bytes;
  make_tiny_draco(bytes);
  if (bytes.empty()) return 100; // failed to generate test asset

  // 2) Decode via the *public* helpers and assert a plausible result
  DracoMDCDecoder* dec = decode_pc(bytes.data(), static_cast<uint32_t>(bytes.size()));
  if (!dec) return 101;

  const auto n = get_n_points(dec);
  if (n == 0) { free_decoder(dec); return 102; }               // must decode to >0 points
  if (get_point_array(dec) == nullptr) { free_decoder(dec); return 103; }
  if (get_color_array(dec) == nullptr)  { free_decoder(dec); return 104; }

  free_decoder(dec);
  return 0;
}
