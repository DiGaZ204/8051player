from machine import Pin, PWM
from umqtt.simple import MQTTClient
import utime, xtools
from machine import UART

# 初始化 UART
com = UART(2, 9600, tx=17, rx=16)
com.init(9600)

# 連接 WiFi 並啟用 LED
xtools.connect_wifi_led()

# 設置 ESP32 的引腳
pin_32 = Pin(26, Pin.OUT)  

# 定義 MQTT 客戶端
client = MQTTClient(
    client_id=xtools.get_id(),  
    server="broker.hivemq.com",  
    ssl=False  
)

# 定義頻率對應的音階
notes = {
    "1L": 131,  # Do (低)
    "2L": 147,  # Re (低)
    "3L": 165,  # Mi (低)
    "4L": 175,  # Fa (低)
    "5L": 196,  # Sol (低)
    "6L": 220,  # La (低)
    "7L": 247,  # Si (低)
    "1": 262,   # Do (中)
    "2": 294,   # Re (中)
    "3": 330,   # Mi (中)
    "4": 349,   # Fa (中)
    "5": 392,   # Sol (中)
    "6": 440,   # La (中)
    "7": 494,   # Si (中)
    "1H": 523,  # Do (高)
    "2H": 587,  # Re (高)
    "3H": 659,  # Mi (高)
    "4H": 698,  # Fa (高)
    "5H": 784,  # Sol (高)
    "6H": 880,  # La (高)
    "7H": 988,   # Si (高)
    "R": 0
}

# 定義持續時間對應的時間長度（秒）
durations = {
    "8": 0.25,  # 八分音符
    "4": 0.5,   # 四分音符
    "2": 1.0,   # 二分音符
    "1": 2.0    # 全音符
}
reverse_dict = {v: k for k, v in durations.items()}

# 定義回撥函數，用於接收 MQTT 訊息
def sub_cb(topic, msg):
    print("收到訊息: ", msg.decode())
    # 將消息拆分成單個音符和時值的組合
    tones = msg.decode().split(',')

    for tone in tones:
        # 檢查是否為休止符（空節拍）
        if tone == "R":
            utime.sleep(durations["4"])  # 默認為四分音符的持續時間
            com.write('R\r\n')
            continue
        elif len(tone) > 1 and tone[:-1] in notes and tone[-1] in durations:
            note = tone[:-1]  # 音符部分
            duration = durations[tone[-1]]  # 時值部分
        elif tone in notes:  # 如果沒有時值，預設為四分音符
            note = tone
            duration = durations["4"]
        else:
            continue  # 如果音符無效，跳過
        
        # 播放音符
        freq = notes[note]  # 獲取音符對應的頻率
        led_pwm32 = PWM(pin_32, freq=freq, duty=512)  # 設置 PWM
        
        # 檢查音符是否在 notes 字典中，如果是，則傳送到 8051
        key = reverse_dict.get(duration)
        if note in notes:
            if key== "4":
                com.write(note+'\r\n')  # 將音符轉換為字串並透過 UART 傳送給 8051
            else:
                com.write(note+key+'\r\n')
                
        utime.sleep(duration)  # 持續時間
        led_pwm32.deinit()  # 停止 PWM
        
#         while com.any():
#             received_data = com.readline()
#             print("從 8051 收到訊息:", received_data)
            # 將 8051 收到的訊息回傳給 ESP32
# 設置 MQTT 客戶端的回撥函數
client.set_callback(sub_cb)
client.connect()  # 連接 MQTT 伺服器

topic = "sensors/livingroom/temp"
print(topic)
client.subscribe(topic)  

# 主循環，檢查 MQTT 訊息
while True:
    client.check_msg()
    
    # 監聽來自 8051 的 UART 訊息
#     if com.any():
#         received_data = com.readline()
#         print("從 8051 收到訊息:", received_data)
