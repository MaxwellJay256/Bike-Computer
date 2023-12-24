# Bluetooth 
## 写在前面  
1. 基于idf的gatt_server_service_table例程修改而来，主要就是对notify对应的部分作了修改，整体框架为不变，要加入相应的功能代码请直接参考第三点。  
2. 采用的是BLE（Bluetooth Low Energy 低功耗蓝牙），好处是在传输数据的链接事件后便会关闭射频模块进入睡眠，但是能接受信号，因而功耗很低。但据网上消息esp32的BLE实际的功耗依然较大，据说接电池用耗电较快。具体的协议及多层封装较为复杂，且idf例程解析很少，网上相关资料也不多，改来改去只能说勉强能用。
## 一、基本协议与原理  
GATT ：Generic Attribute Profile Specification（通用属性配置文件规范）

在通过GAP（Generic Access Profile 通用访问规范）建立了蓝牙连接后，GATT规定了蓝牙设备传输数据的规则与格式，即以server and client （服务端与客户端）的模式进行通信（一个外设只能连接一个client，本工程中手机就是client，esp32作为server），且所有的通信事件都由client发起。

GATT事务的结构由Profiles，Services和Characteristics三层组成。   
* profile  
    * service_1
        * characteristic_1
        * characteristic_2
        * characteristic_3
    * service_2
        * characteristic_1
        * characteristic_2  
  
**Profile:**  主要是作为Service的一个集合，便于管理，比如可以把传感器信息、摔倒检测分别作为Service结合在一个Profile里面  
**Service:** 即把数据分为逻辑项，可以包含一个或多个Characteristic  
**Characteristic:** 即最小的数据单元，可以用来传递比如传感器信息等  

大致的通信过程即esp32发送广播，手机端扫描并连接，此时esp32关闭广播，只与手机连接通信，根据对应的协议约定好的时间间隔进行快速的数据传递。

BLE服务端传输消息主要是用notification（即notify）和indication（indicate），前者发送信息无需应答，后者需要应答且如无应答则不能继续发送消息，相对而言数据传输更可靠，但我们的工程中notify应该不会发生丢失数据的情况，遂采用notify。


## 二、代码结构解析   
在idf的例程Gatt_Server_Service_Table的tutorial文件夹下有说明，但仍冗杂，下面仅介绍大致的框架结构，同时各个函数也写了一些注释。

### 服务表（Service Table）  
每个服务对应一个服务表，表的第一个元素是服务声明，随后是特征的声明，第三个是特征值，值为temp2，第四个元素使客户端特性配置描述符，如果还要添加特征则在表中仿照第二三四的格式加入对应的配置。
```c
enum
{
    IDX_SVC,

    IDX_CHAR_TEMP,
    IDX_CHAR_VAL_TEMP,
    IDX_CHAR_CFG_TEMP,
    
    HRS_IDX_NB,
};

static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] =
{
        // Service Declaration
        [IDX_SVC] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(GATTS_SERVICE_UUID_TEST), (uint8_t *)&GATTS_SERVICE_UUID_TEST}},

        /* Characteristic Declaration */
        // 读及通知
        [IDX_CHAR_TEMP] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_notify}},

        /* Characteristic Value */
        [IDX_CHAR_VAL_TEMP] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_TEST_TEMP, ESP_GATT_PERM_READ, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(temp2), (uint8_t *)temp2}},

        /* Client Characteristic Configuration Descriptor */
        [IDX_CHAR_CFG_TEMP] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(uint16_t), sizeof(temp1), (uint8_t *)temp1}},
};
```
如果要新建一个服务表，则
在gatts_profile_event_handler中的ESP_GATTS_REG_EVT 的case中的esp_ble_gatts_create_attr_tab里进行注册，同时还要在ESP_GATTS_CREAT_ATTR_TAB_EVT的case中用esp_ble_gatts_start_service开启服务


### 主入口  
在bluetooth_init()函数中，在主程序中只需调用该函数，实际的功能函数就在Bluetooth.c里加入和修改即可
```c
void bluetooth_init()
{
    esp_err_t ret;

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed\n", __func__);
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed\n", __func__);
        return;
    }

    ESP_LOGI(GATTS_TABLE_TAG, "%s init bluetooth\n", __func__);
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed\n", __func__);
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed\n", __func__);
        return;
    }

    esp_ble_gatts_register_callback(gatts_event_handler);
    esp_ble_gap_register_callback(gap_event_handler);
    esp_ble_gatts_app_register(ESP_HEART_RATE_APP_ID);
    return;
}
```  
### 设置应用Profile  
设置Profile ID，Profile_tab（Profile表）

### 设置GAP参数  
配置 adv_data（广播数据） 和 scan_rsp_data（扫描响应数据） 的参数，设置
### GAP 事件处理  
主要是开始广播，建立连接等
### GATT 事件处理  
本项目中主要用到的是Write event，即手机端主动打开notify，esp32便不断发送消息。

### 创建 Service和Characteristic 和 开始服务  



## 三、修改使用
要修改的地方：  
gatts_profile_event_handler函数中的ESP_GATTS_WRITE_EVT case   
手机端的操作即连接蓝牙后在任意app或者小程序中开启对应服务的notify，便可持续接收到消息 
```c
case ESP_GATTS_WRITE_EVT:
        if (!param->write.is_prep)
        {
            // the data length of gattc write  must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX.
            ESP_LOGI(GATTS_TABLE_TAG, "GATT_WRITE_EVT, handle = %d, value len = %d, value :", param->write.handle, param->write.len);
            esp_log_buffer_hex(GATTS_TABLE_TAG, param->write.value, param->write.len);
            // 如果写入的句柄是handle_table[IDX_CHAR_CFG_TEMP]
            if (handle_table[IDX_CHAR_CFG_TEMP] == param->write.handle && param->write.len == 2)
            {
                //获取描述符，并据此进行不同的操作
                uint16_t descr_value = param->write.value[1] << 8 | param->write.value[0];
                // 如果descr_value为0x0001，表示notify使能，在此处可以创建任务发送传感器数据
                if (descr_value == 0x0001)
                {
                    ESP_LOGI(GATTS_TABLE_TAG, "notify enable");
                    xTaskCreate(get_temp, "get temp", 8192, NULL, 10, pTask);
                    notify_flag=true;
                }
                // 如果descr_value为0x0002，表示indicate使能,此处未用到
                else if (descr_value == 0x0002)
                {
                    ESP_LOGI(GATTS_TABLE_TAG, "indicate enable");
                    uint8_t indicate_data[15];
                    for (int i = 0; i < sizeof(indicate_data); ++i)
                    {
                        indicate_data[i] = i % 0xff;
                    }
                    // the size of indicate_data[] need less than MTU size
                }
                // 如果descr_value为0x0000，表示notify/indicate disable
                else if (descr_value == 0x0000)
                {
                    ESP_LOGI(GATTS_TABLE_TAG, "notify/indicate disable ");
                    notify_flag=false;
                    //vTaskDelete(pTask);
                }
                else
                {
                    ESP_LOGE(GATTS_TABLE_TAG, "unknown descr value");
                    esp_log_buffer_hex(GATTS_TABLE_TAG, param->write.value, param->write.len);
                }
            }
            /* send response when param->write.need_rsp is true*/
            if (param->write.need_rsp)
            {
                esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
            }
        }
        else
        {
            /* handle prepare write */
            example_prepare_write_event_env(gatts_if, &prepare_write_env, param);
        }
        break;
```  
其中创建的任务函数思路大致如下：  
```c
/// @brief 摔倒检测并发送数据
static void get_temp(void *arg)
{
    while (1)
    {
        if (notify_flag == true)
        {
            uint8_t temp = 20 + rand() % 11; //用随机数模拟数据
            uint8_t normal[] = "everything is ok";
            uint8_t warning[] = "fall detected!";
            
            if(temp > 28)//!此处可根据mpu6050计算出的姿态值进行判断，或者修改实现传感器数据的传输，甚至可以加入蜂鸣器进行报警
            {
               esp_ble_gatts_set_attr_value(handle_table[IDX_CHAR_VAL_TEMP], sizeof(warning), warning);
               esp_ble_gatts_send_indicate(profile_tab[0].gatts_if, profile_tab[0].conn_id,handle_table[IDX_CHAR_VAL_TEMP], sizeof(warning), warning, false);//发送数据
            }
            else
            {
                esp_ble_gatts_set_attr_value(handle_table[IDX_CHAR_VAL_TEMP], sizeof(normal), normal); 
                esp_ble_gatts_send_indicate(profile_tab[0].gatts_if, profile_tab[0].conn_id,handle_table[IDX_CHAR_VAL_TEMP], sizeof(normal), normal, false);//发送数据
            }
        }
        else
        {
            vTaskDelete(NULL);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
```  
可根据实际的传感器函数接口和需求修改或重写。

## 参考资料和教程：  
https://developer.aliyun.com/article/769552#  
https://juejin.cn/post/7169827441974706213   
https://blog.csdn.net/coder_jun/article/details/130648445  
https://blog.csdn.net/qq_43940227/article/details/132721717