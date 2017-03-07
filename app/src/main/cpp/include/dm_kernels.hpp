#ifndef DM_KERNELS_HPP
#define DM_KERNELS_HPP

char * common_cl = "#ifndef PRECISION\n"
        "  #define PRECISION 32      // Data-types: half, single or double precision, complex or regular\n"
        "#endif\n"
        "\n"
        "#if PRECISION == 16\n"
        "  #pragma OPENCL EXTENSION cl_khr_fp16: enable\n"
        "#endif\n"
        "\n"
        "// Half-precision\n"
        "#if PRECISION == 16\n"
        "  typedef half real;\n"
        "  typedef half2 real2;\n"
        "  typedef half4 real4;\n"
        "  typedef half8 real8;\n"
        "  typedef half16 real16;\n"
        "  #define ZERO 0\n"
        "  #define ONE 1\n"
        "  #define SMALLEST -1.0e14\n"
        "\n"
        "// Single-precision\n"
        "#elif PRECISION == 32\n"
        "  typedef float real;\n"
        "  typedef float2 real2;\n"
        "  typedef float4 real4;\n"
        "  typedef float8 real8;\n"
        "  typedef float16 real16;\n"
        "  #define ZERO 0.0f\n"
        "  #define ONE 1.0f\n"
        "  #define SMALLEST -1.0e37f\n"
        "#endif\n"
        "\n"
        "#ifndef VWM\n"
        "    #define VWM 4\n"
        "#endif\n"
        "\n"
        "#ifdef VWM\n"
        "    #if VWM == 1\n"
        "        typedef real realM;\n"
        "        #define vload vloadM;\n"
        "    #elif VWM == 2\n"
        "        typedef real2 realM;\n"
        "        #define vload2 vloadM;\n"
        "    #elif VWM == 4\n"
        "        typedef real4 realM;\n"
        "        #define vload4 vloadM;\n"
        "    #elif VWM == 8\n"
        "        typedef real8 realM;\n"
        "        #define vload8 vloadM;\n"
        "    #elif VWM == 16\n"
        "        typedef real16 realM;\n"
        "        #define vload16 vloadM;\n"
        "    #endif\n"
        "#endif\n"
        "\n"
        "#ifndef USE_CL_MAD\n"
        "  #define USE_CL_MAD 0\n"
        "#endif\n"
        "\n"
        "static inline int getIndexFrom3D(int d1, int d2, int d3, int i1, int i2, int i3) {\n"
        "\treturn i1 * (d2 * d3) + i2 * d3 + i3;\n"
        "}\n"
        "\n"
        "static inline int getIndexFrom4D(int d1, int d2, int d3, int d4, int i1, int i2, int i3, int i4) {\n"
        "\treturn i1 * (d2 * d3 * d4) + i2 * (d3 * d4) + i3 * d4 + i4;\n"
        "}\n"
        "\n"
        "#if PRECISION == 16\n"
        "__kernel void convertFloatToHalf(\n"
        "    __global const float *input,\n"
        "    __global half *output) {\n"
        "    int idx = get_global_id(0);\n"
        "    vstore_half(input[idx], 0, &output[idx]);\n"
        "}\n"
        "\n"
        "__kernel void convertHalfToFloat(\n"
        "    __global const half *input,\n"
        "    __global float *output) {\n"
        "    int idx = get_global_id(0);\n"
        "    //output[idx] = convert_float(input[idx]);\n"
        "    output[idx] = (float)input[idx];\n"
        "}\n"
        "#endif\n";

char * im2col_cl = "__kernel void caffe_im2col(const int n,\n"
        "                                     __global const real* data_im,\n"
        "                                     const int data_im_off,\n"
        "                                     const int height, const int width,\n"
        "                                     const int kernel_h,\n"
        "                                     const int kernel_w, const int pad_h,\n"
        "                                     const int pad_w, const int stride_h,\n"
        "                                     const int stride_w,\n"
        "                                     const int dilation_h,\n"
        "                                     const int dilation_w,\n"
        "                                     const int height_col,\n"
        "                                     const int width_col,\n"
        "                                     __global real* data_col,\n"
        "                                     const int data_col_off) {\n"
        "\n"
        "  for (int index = get_global_id(0); index < n;\n"
        "      index += get_global_size(0)) {\n"
        "    const int h_index = index / width_col;\n"
        "    const int h_col = h_index % height_col;\n"
        "    const int w_col = index % width_col;\n"
        "    const int c_im = h_index / height_col;\n"
        "    const int c_col = c_im * kernel_h * kernel_w;\n"
        "    const int h_offset = h_col * stride_h - pad_h;\n"
        "    const int w_offset = w_col * stride_w - pad_w;\n"
        "    __global real* data_col_ptr = data_col + data_col_off;\n"
        "    data_col_ptr += (c_col * height_col + h_col) * width_col + w_col;\n"
        "    __global const real* data_im_ptr = data_im + data_im_off;\n"
        "    data_im_ptr += (c_im * height + h_offset) * width + w_offset;\n"
        "    for (int i = 0; i < kernel_h; ++i) {\n"
        "      for (int j = 0; j < kernel_w; ++j) {\n"
        "        int h_im = h_offset + i * dilation_h;\n"
        "        int w_im = w_offset + j * dilation_w;\n"
        "        *data_col_ptr =\n"
        "            (h_im >= 0 && w_im >= 0 && h_im < height && w_im < width) ?\n"
        "                data_im_ptr[i * dilation_h * width + j * dilation_w] : 0;\n"
        "        data_col_ptr += height_col * width_col;\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}\n"
        "\n"
        "__kernel void caffe_col2im(const int n,\n"
        "                                     __global const real* data_col,\n"
        "                                     const int data_col_off,\n"
        "                                     const int height, const int width,\n"
        "                                     const int channels,\n"
        "                                     const int kernel_h,\n"
        "                                     const int kernel_w, const int pad_h,\n"
        "                                     const int pad_w, const int stride_h,\n"
        "                                     const int stride_w,\n"
        "                                     const int dilation_h,\n"
        "                                     const int dilation_w,\n"
        "                                     const int height_col,\n"
        "                                     const int width_col,\n"
        "                                     __global real* data_im,\n"
        "                                     const int data_im_off) {\n"
        "\n"
        "  for (int index = get_global_id(0); index < n; index += get_global_size(0)) {\n"
        "    real val = 0;\n"
        "    const int w_im = index % width + pad_w;\n"
        "    const int h_im = (index / width) % height + pad_h;\n"
        "    const int c_im = index / (width * height);\n"
        "    int kernel_extent_w = (kernel_w - 1) * dilation_w + 1;\n"
        "    int kernel_extent_h = (kernel_h - 1) * dilation_h + 1;\n"
        "    // compute the start and end of the output\n"
        "    const int w_col_start =\n"
        "        (w_im < kernel_extent_w) ? 0 : (w_im - kernel_extent_w) / stride_w + 1;\n"
        "    const int w_col_end = min(w_im / stride_w + 1, width_col);\n"
        "    const int h_col_start =\n"
        "        (h_im < kernel_extent_h) ? 0 : (h_im - kernel_extent_h) / stride_h + 1;\n"
        "    const int h_col_end = min(h_im / stride_h + 1, height_col);\n"
        "    // TODO: use LCM of stride and dilation to avoid unnecessary loops\n"
        "    for (int h_col = h_col_start; h_col < h_col_end; h_col += 1) {\n"
        "      for (int w_col = w_col_start; w_col < w_col_end; w_col += 1) {\n"
        "        int h_k = (h_im - h_col * stride_h);\n"
        "        int w_k = (w_im - w_col * stride_w);\n"
        "        if (h_k % dilation_h == 0 && w_k % dilation_w == 0) {\n"
        "          h_k /= dilation_h;\n"
        "          w_k /= dilation_w;\n"
        "          int data_col_index = (((c_im * kernel_h + h_k) * kernel_w + w_k) *\n"
        "                                height_col + h_col) * width_col + w_col;\n"
        "          val += data_col[data_col_off + data_col_index];\n"
        "        }\n"
        "      }\n"
        "    }\n"
        "    data_im[data_im_off + index] = val;\n"
        "  }\n"
        "}";

std::vector<std::string> kernels {
        std::string(common_cl),
        std::string(im2col_cl)
};

#endif