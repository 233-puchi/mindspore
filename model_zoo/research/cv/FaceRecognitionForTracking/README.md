# Contents

- [Face Recognition For Tracking Description](#face-recognition-for-tracking-description)
- [Model Architecture](#model-architecture)
- [Dataset](#dataset)
- [Environment Requirements](#environment-requirements)  
- [Script Description](#script-description)
    - [Script and Sample Code](#script-and-sample-code)
    - [Running Example](#running-example)
- [Model Description](#model-description)
    - [Performance](#performance)  
- [ModelZoo Homepage](#modelzoo-homepage)

# [Face Recognition For Tracking Description](#contents)

This is a face recognition for tracking network based on Resnet, with support for training and evaluation on Ascend910, GPU and CPU.

ResNet (residual neural network) was proposed by Kaiming He and other four Chinese of Microsoft Research Institute. Through the use of ResNet unit, it successfully trained 152 layers of neural network, and won the championship in ilsvrc2015. The error rate on top 5 was 3.57%, and the parameter quantity was lower than vggnet, so the effect was very outstanding. Traditional convolution network or full connection network will have more or less information loss. At the same time, it will lead to the disappearance or explosion of gradient, which leads to the failure of deep network training. ResNet solves this problem to a certain extent. By passing the input information to the output, the integrity of the information is protected. The whole network only needs to learn the part of the difference between input and output, which simplifies the learning objectives and difficulties.The structure of ResNet can accelerate the training of neural network very quickly, and the accuracy of the model is also greatly improved. At the same time, ResNet is very popular, even can be directly used in the concept net network.

[Paper](https://arxiv.org/pdf/1512.03385.pdf):  Kaiming He, Xiangyu Zhang, Shaoqing Ren, Jian Sun. "Deep Residual Learning for Image Recognition"

# [Model Architecture](#contents)

Face Recognition For Tracking uses a Resnet network for performing feature extraction.

# [Dataset](#contents)

We use about 10K face images as training dataset and 2K as evaluating dataset in this example, and you can also use your own datasets or open source datasets (e.g. Labeled Faces in the Wild)
The directory structure is as follows:

```python
.
└─ dataset
  ├─ train dataset
    ├─ ID1
      ├─ ID1_0001.jpg
      ├─ ID1_0002.jpg
      ...
    ├─ ID2
      ...
    ├─ ID3
      ...
    ...
  ├─ test dataset
    ├─ ID1
      ├─ ID1_0001.jpg
      ├─ ID1_0002.jpg
      ...
    ├─ ID2
      ...
    ├─ ID3
      ...
    ...
```

# [Environment Requirements](#contents)

- Hardware(Ascend/GPU/CPU)
    - Prepare hardware environment with Ascend processor.
- Framework
    - [MindSpore](https://www.mindspore.cn/install/en)
- For more information, please check the resources below:
    - [MindSpore tutorials](https://www.mindspore.cn/tutorial/training/en/master/index.html)
    - [MindSpore Python API](https://www.mindspore.cn/doc/api_python/en/master/index.html)

# [Script Description](#contents)

## [Script and Sample Code](#contents)

The entire code structure is as following:

```python
.
└─ Face Recognition For Tracking
  ├─ README.md
  ├─ scripts
    ├─ run_standalone_train.sh              # launch standalone training(1p) in ascend
    ├─ run_distribute_train.sh              # launch distributed training(8p) in ascend
    ├─ run_eval.sh                          # launch evaluating in ascend
    ├─ run_export.sh                        # launch exporting air/mindir model
    ├─ run_standalone_train_gpu.sh          # launch standalone training(1p) in gpu
    ├─ run_distribute_train_gpu.sh          # launch distributed training(8p) in gpu
    ├─ run_eval_gpu.sh                      # launch evaluating in gpu
    ├─ run_export_gpu.sh                    # launch exporting mindir model in gpu
    ├─ run_train_cpu.sh                     # launch standalone training in cpu
    ├─ run_eval_cpu.sh                      # launch evaluating in cpu
    └─ run_export_cpu.sh                    # launch exporting mindir model in cpu
  ├─ src
    ├─ config.py                            # parameter configuration
    ├─ dataset.py                           # dataset loading and preprocessing for training
    ├─ reid.py                              # network backbone
    ├─ log.py                               # log function
    ├─ loss.py                              # loss function
    ├─ lr_generator.py                      # generate learning rate
    └─ me_init.py                           # network initialization
  ├─ train.py                               # training scripts
  ├─ eval.py                                # evaluation scripts
  └─ export.py                              # export air/mindir model
```

## [Running Example](#contents)

### Train

- Stand alone mode

    ```bash
    Ascend:

    cd ./scripts
    sh run_standalone_train.sh [DATA_DIR] [USE_DEVICE_ID]
    ```

    ```bash
    GPU:

    cd ./scripts
    sh run_standalone_train_gpu.sh [DATA_DIR]
    ```

    ```bash
    CPU:

    cd ./scripts
    sh run_train_cpu.sh [DATA_DIR]
    ```

    or (fine-tune)

    ```bash
    Ascend:

    cd ./scripts
    sh run_standalone_train.sh [DATA_DIR] [USE_DEVICE_ID] [PRETRAINED_BACKBONE]
    ```

    ```bash
    GPU:

    cd ./scripts
    sh run_standalone_train.sh [DATA_DIR] [PRETRAINED_BACKBONE]
    ```

    ```bash
    CPU:

    cd ./scripts
    sh run_train.sh [DATA_DIR] [PRETRAINED_BACKBONE]
    ```

    for example, on Ascend:

    ```bash
    cd ./scripts
    sh run_standalone_train.sh /home/train_dataset 0 /home/a.ckpt
    ```

- Distribute mode (recommended)

    ```bash
    Ascend:

    cd ./scripts
    sh run_distribute_train.sh [DATA_DIR] [RANK_TABLE]
    ```

    ```bash
    GPU:

    cd ./scripts
    sh run_distribute_train_gpu.sh [DEVICE_NUM] [VISIBLE_DEVICES(0, 1, 2, 3, 4, 5, 6, 7)] [DATASET_PATH]
    ```

    or (fine-tune)

    ```bash
    Ascend:

    cd ./scripts
    sh run_distribute_train.sh [DATA_DIR] [RANK_TABLE] [PRETRAINED_BACKBONE]
    ```

    ```bash
    GPU:

    cd ./scripts
    sh run_distribute_train_gpu.sh [DEVICE_NUM] [VISIBLE_DEVICES(0, 1, 2, 3, 4, 5, 6, 7)] [DATASET_PATH] [PRE_TRAINED]
    ```

    for example:

    ```bash
    cd ./scripts
    sh run_distribute_train.sh /home/train_dataset ./rank_table_8p.json /home/a.ckpt
    ```

You will get the loss value of each step as following in "./output/[TIME]/[TIME].log" or "./scripts/device0/train.log":

```python
epoch[0], iter[10], loss:43.314265, 8574.83 imgs/sec, lr=0.800000011920929
epoch[0], iter[20], loss:45.121095, 8915.66 imgs/sec, lr=0.800000011920929
epoch[0], iter[30], loss:42.342847, 9162.85 imgs/sec, lr=0.800000011920929
epoch[0], iter[40], loss:39.456583, 9178.83 imgs/sec, lr=0.800000011920929

...
epoch[179], iter[14900], loss:1.651353, 13001.25 imgs/sec, lr=0.02500000037252903
epoch[179], iter[14910], loss:1.532123, 12669.85 imgs/sec, lr=0.02500000037252903
epoch[179], iter[14920], loss:1.760322, 13457.81 imgs/sec, lr=0.02500000037252903
epoch[179], iter[14930], loss:1.694281, 13417.38 imgs/sec, lr=0.02500000037252903
```

### Evaluation

```bash
Ascend:

cd ./scripts
sh run_eval.sh [EVAL_DIR] [USE_DEVICE_ID] [PRETRAINED_BACKBONE]
```

```bash
GPU:

cd ./scripts
sh run_eval_gpu.sh [EVAL_DIR] [PRETRAINED_BACKBONE]
```

```bash
CPU:

cd ./scripts
sh run_eval_cpu.sh [EVAL_DIR] [PRETRAINED_BACKBONE]
```

for example, on Ascend:

```bash
cd ./scripts
sh run_eval.sh /home/test_dataset 0 /home/a.ckpt
```

You will get the result as following in "./scripts/device0/eval.log" or txt file in [PRETRAINED_BACKBONE]'s folder:

```python
0.5: 0.9273788254649683@0.020893691253149882
0.3: 0.8393850978779193@0.07438552515516506
0.1: 0.6220871197028316@0.1523084478903911
0.01: 0.2683641598437038@0.26217882879427634
0.001: 0.11060269148211463@0.34509718987101223
0.0001: 0.05381678898728808@0.4187797093636618
1e-05: 0.035770748447963394@0.5053771466191392
```

### Convert model

If you want to infer the network on Ascend 310, you should convert the model to AIR:

```bash
Ascend:

cd ./scripts
sh run_export.sh [BATCH_SIZE] [USE_DEVICE_ID] [PRETRAINED_BACKBONE]
```

Or if you would like to convert your model to MINDIR file on GPU or CPU:

```bash
GPU:

cd ./scripts
sh run_export_gpu.sh [PRETRAINED_BACKBONE] [BATCH_SIZE] [FILE_NAME](optional)
```

```bash
CPU:

cd ./scripts
sh run_export_cpu.sh [PRETRAINED_BACKBONE] [BATCH_SIZE] [FILE_NAME](optional)
```

# [Model Description](#contents)

## [Performance](#contents)

### Training Performance

| Parameters                 | Ascend                 |GPU    |CPU    |
| -------------------------- | ----------------------------------------------------------- | ----------------------------------------------------------- | ----------------------------------------------------------- |
| Model Version              | V1         | V1      | V1    |
| Resource            | Ascend 910; CPU 2.60GHz, 192cores; Memory, 755G; OS Euler2.8       |Tesla V100-PCIE       |Intel(R) Xeon(R) CPU E5-2690 v4       |
| uploaded Date              | 09/30/2020 (month/day/year)  |04/17/2021 (month/day/year)      |04/17/2021 (month/day/year)                                 |
| MindSpore Version          | 1.0.0          | 1.2.0     |1.2.0               |
| Dataset                    | 10K images           | 10K images         | 10K images       |
| Training Parameters        | epoch=180, batch_size=16, momentum=0.9                      | epoch=40, batch_size=128(1p); 16(8p), momentum=0.9                      | epoch=40, batch_size=128, momentum=0.9                      |
| Optimizer                  | SGD         | SGD   | SGD   |
| Loss Function              | Softmax Cross Entropy        | Softmax Cross Entropy   | Softmax Cross Entropy            |
| outputs     | probability              | probability        |probability     |
| Speed                      | 1pc: 8-10 ms/step; 8pcs: 9-11 ms/step                       | 1pc: 30 ms/step; 8pcs: 20 ms/step                | 1pc: 2.5 s/step    |
| Total time                 | 1pc: 1 hour; 8pcs: 0.1 hours           | 1pc: 2 minutes; 8pcs: 1.5 minutes                    |1pc: 2 hours    |
| Checkpoint for Fine tuning | 17M (.ckpt file)                 | 17M (.ckpt file)                 | 17M (.ckpt file)                 |

### Evaluation Performance

| Parameters          |Ascend     |GPU           |CPU           |
| ------------------- | --------------------------- | --------------------------- | --------------------------- |
| Model Version       |V1            |V1   |V1 |
| Resource            | Ascend 910; OS Euler2.8                  |Tesla V100-PCIE                 |Intel(R) Xeon(R) CPU E5-2690 v4        |
| Uploaded Date       | 09/30/2020 (month/day/year) | 04/17/2021 (month/day/year) | 04/17/2021 (month/day/year) |
| MindSpore Version   | 1.0.0                       | 1.2.0                       |1.2.0                       |
| Dataset             | 2K images                   | 2K images                   | 2K images                   |
| batch_size          | 128                         | 128                         |128                         |
| outputs             | recall                      | recall                      |recall                      |
| Recall       | 0.62(FAR=0.1)               | 0.62(FAR=0.1)               | 0.62(FAR=0.1)               |
| Model for inference | 17M (.ckpt file)            | 17M (.ckpt file)            | 17M (.ckpt file)            |

# [ModelZoo Homepage](#contents)

Please check the official [homepage](https://gitee.com/mindspore/mindspore/tree/master/model_zoo).
