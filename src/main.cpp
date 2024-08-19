#include <Arduino.h> // 包含Arduino库

#include <WiFi.h> // 包含WiFi库

#include <PubSubClient.h> // 包含PubSubClient库

#include <Ticker.h> // 包含Ticker库

#define LED 2                                                                                        // 定义LED引脚为2
#define BUTTON 4                                                                                     // 定义按钮引脚为4
#define ONENET_TOPIC_PROP_POST "$sys/" products_id "/" devices_id "/thing/property/post"             // 设备属性上报请求
#define ONENET_TOPIC_PROP_SET "$sys/" products_id "/" devices_id "/thing/property/set"               // 设备属性设置请求
#define ONENET_TOPIC_PROP_POST_REPLY "$sys/" products_id "/" devices_id "/thing/property/post/reply" // 设备属性上报响应
#define ONENET_TOPIC_PROP_SET_REPLY "$sys/" products_id "/" devices_id "/thing/property/set_reply"   // 设备属性设置响应
#define ONENET_TOPIC_PROP_FORMAT "{\"id\":\"%u\",\"version\":\"1.0\",\"params\":%s}"                 // 设备属性模板格式

int postMsgId = 0; // 消息ID

const char *ssid = "赵今麦的小米13"; // WiFi名称

const char *password = "66666666"; // WiFi密码

const char *mqtt_server = "mqtts.heclouds.com"; // MQTT服务器地址

const int mqtt_port = 1883; // MQTT服务器端口

#define products_id "你的产品ID" // 产品ID

#define devices_id "你的设备ID" // 设备ID

#define token " 你的token" // 设备token

float gps = 30.0; // GPS坐标变量

WiFiClient espClient;           // 创建一个WiFiClient对象
PubSubClient client(espClient); // 创建一个PubSubClient对象

Ticker ticker; // 创建一个Ticker对象

void LED_status(int time) // LED状态函数
{
  digitalWrite(LED, HIGH); // 点亮LED
  delay(time);             // 延时
  digitalWrite(LED, LOW);  // 熄灭LED
  delay(time);             // 延时
}

void WiFi_Connect() // WiFi连接函数
{

  while (WiFi.status() != WL_CONNECTED) // 检查WiFi连接状态
  {
    LED_status(500);                        // 闪烁LED
    Serial.println("Connecting to WiFi.."); // 打印连接WiFi信息
  }
  Serial.println("Connected to the WiFi network"); // 打印连接成功信息
  Serial.println(WiFi.localIP());                  // 打印本地IP地址
  digitalWrite(LED, HIGH);                         // 点亮LED
}

void OneNet_Connect() // OneNet连接函数
{

  client.setServer(mqtt_server, mqtt_port);       // 设置MQTT服务器地址和端口
  client.connect(devices_id, products_id, token); // 连接OneNet
  if (client.connected())                         // 检查连接状态
  {
    LED_status(100);                         // 闪烁LED
    Serial.println("Connecting to OneNet!"); // 打印连接成功信息
  }
  else
  {
    Serial.println("Failed to connect to OneNet"); // 打印连接失败信息
  }

  client.subscribe(ONENET_TOPIC_PROP_SET);        // 订阅设备属性设置请求
  client.subscribe(ONENET_TOPIC_PROP_POST_REPLY); // 订阅设备属性上报响应
}

void OneNet_Prop_Post() // OneNet属性上报函数
{
  if (client.connected()) // 检查连接状态
  {
    char parmas[256];                                                // 参数缓冲区
    char jsonBuf[256];                                               // JSON缓冲区
    sprintf(parmas, "{\"gps\":{\"value\":%.1f}}", gps);              // 格式化参数
    Serial.println(parmas);                                          // 打印参数
    sprintf(jsonBuf, ONENET_TOPIC_PROP_FORMAT, postMsgId++, parmas); // 格式化JSON
    Serial.println(jsonBuf);                                         // 打印JSON
    if (client.publish(ONENET_TOPIC_PROP_POST, jsonBuf))             // 发布属性
    {
      LED_status(100);                          // 闪烁LED
      Serial.println("Post property success!"); // 打印成功信息
    }
    else
    {
      Serial.println("Post property failed"); // 打印失败信息
    }
  }
}

void setup() // 初始化函数
{

  Serial.begin(9600);            // 初始化串口
  pinMode(LED, OUTPUT);          // 设置LED引脚为输出
  pinMode(BUTTON, INPUT_PULLUP); // 设置按钮引脚为输入
  WiFi.begin(ssid, password);    // 连接WiFi
  WiFi_Connect();                // 连接WiFi
  OneNet_Connect();              // 连接OneNet

  ticker.attach(5, OneNet_Prop_Post); // 设置定时器
}

void loop() // 主循环函数
{
  if (WiFi.status() != WL_CONNECTED) // 检查WiFi连接状态
  {
    WiFi_Connect(); // 重新连接WiFi
  }
  if (!client.connected()) // 检查OneNet连接状态
  {
    OneNet_Connect(); // 重新连接OneNet
  }

  if (digitalRead(BUTTON) == 0) // 检查按钮状态
  {
    gps += 1; // 增加GPS值
  }

  client.loop(); // 处理MQTT消息
}