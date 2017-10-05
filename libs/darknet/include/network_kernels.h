
#include "cuda_runtime.h"
#include "curand.h"
#include "cublas_v2.h"


#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "network.h"
#include "image.h"
#include "data.h"
#include "utils.h"
#include "parser.h"

#include "crop_layer.h"
#include "connected_layer.h"
#include "rnn_layer.h"
#include "gru_layer.h"
#include "crnn_layer.h"
#include "detection_layer.h"
#include "region_layer.h"
#include "convolutional_layer.h"
#include "activation_layer.h"
#include "maxpool_layer.h"
#include "reorg_layer.h"
#include "avgpool_layer.h"
#include "normalization_layer.h"
#include "batchnorm_layer.h"
#include "cost_layer.h"
#include "local_layer.h"
#include "softmax_layer.h"
#include "dropout_layer.h"
#include "route_layer.h"
#include "shortcut_layer.h"
#include "blas.h"


float * get_network_output_layer_gpu(network net, int i);
float * get_network_delta_gpu_layer(network net, int i);
float * get_network_output_gpu(network net);

void forward_network_gpu(network net, network_state state);

void backward_network_gpu(network net, network_state state);

void update_network_gpu(network net);

void forward_backward_network_gpu(network net, float *x, float *y);

float train_network_datum_gpu(network net, float *x, float *y);

typedef struct {
    network net;
    data1 d;
    float *err;
} train_args;

void *train_thread(void *ptr);

pthread_t train_network_in_thread(network net, data1 d, float *err);

void pull_updates(layer l);

void push_updates(layer l);

void update_layer(layer l, network net);

void merge_weights(layer l, layer base);

void scale_weights(layer l, float s);

void pull_weights(layer l);

void push_weights(layer l);

void distribute_weights(layer l, layer base);
void merge_updates(layer l, layer base);
void distribute_updates(layer l, layer base);
void sync_layer(network *nets, int n, int j);

typedef struct{
    network *nets;
    int n;
    int j;
} sync_args;

void *sync_layer_thread(void *ptr);
pthread_t sync_layer_in_thread(network *nets, int n, int j);
void sync_nets(network *nets, int n, int interval);
float train_networks(network *nets, int n, data1 d, int interval);
float *get_network_output_layer_gpu(network net, int i);
float *get_network_output_gpu(network net);
float *network_predict_gpu(network net, float *input);
