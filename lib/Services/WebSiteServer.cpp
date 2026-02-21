#include "WebSiteServer.h"

static httpd_handle_t Webserver = NULL;     // HTTP Server 句柄

/**
 * @brief 存放网页 HTML 的字符串
 */
static const char cBasicWebSite[] = R"=====(

    <!DOCTYPE html>
    <html>
    <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>legged wheel robot web ctrl</title>
    <style>
        *{box-sizing:border-box;margin:0;padding:0}
        body{display:flex;flex-direction:column;align-items:center;min-height:100vh;background-color:#fff;font-family:'Segoe UI',Arial,sans-serif;padding:15px;overflow:hidden;width:100vw}
        .page-title{color:#333;margin-bottom:15px;text-align:center;font-weight:600;font-size:1.3rem;width:100%}
        .connection-status{margin-bottom:15px;padding:6px 12px;border-radius:4px;font-weight:500;font-size:14px}
        .status-connected{background-color:#e8f5e9;color:#2e7d32}
        .status-disconnected{background-color:#ffebee;color:#c62828}
        .btn{display:inline-block;font-size:15px;width:100px;height:60px;text-align:center;border-radius:12px;background-color:#fff;color:#6495ed;border:3px solid #6495ed;padding:0;transition:all .3s;margin:10px 0}
        .row{display:flex;width:100%;justify-content:space-between;align-items:center;gap:10px}
        .columnLateral{flex:1;min-width:0;text-align:center}
        #joy1Div,#joy2Div{width:100%;max-width:200px;height:200px;margin:10px auto}
        @media (max-width:360px){.page-title{font-size:1.1rem}
        #joy1Div,#joy2Div{height:160px}
        .btn{width:80px;height:50px;font-size:14px}
        }
    </style>
    </head>

    <body onload="socket_init()">
    <h1 class="page-title">WLROBOT WiFi遥控模式</h1>
    <div class="connection-status status-disconnected" id="connectionStatus">未连接</div>

    <center>
    <div class="row">
        <div class="columnLateral">
        <div id="joy2Div" style="width:200px;height:200px;margin:10px"></div>
        <div id="joy2Info">
            <span>高度: <span id="joy2YValue">30</span></span><br>
            <span>倾角: <span id="joy2XValue">0</span></span>  
        </div>
        </div>
    
        <div class="columnLateral">
        <div id="joy1Div" style="width:200px;height:200px;margin:10px"></div>
        <div id="joy1Info">
            <span>前后: <span id="joy1YValue">0</span></span><br>
            <span>左右: <span id="joy1XValue">0</span></span>
        </div>
        </div>
    </div>

    <div class="buttons">
        <button class="btn" id="jump">Jump</button>
    </div>
    </center>

    <script>
    // ===== WebSocket 初始化 =====
    var socket;
    function socket_init(){
        try {
            const wsProtocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
            socket = new WebSocket(wsProtocol + '//' + window.location.hostname + '/ws');
            socket.binaryType = "arraybuffer";
            socket.onopen = function(){ updateConnectionStatus(true); }
            socket.onclose = function(){ updateConnectionStatus(false); setTimeout(socket_init,3000); }
            socket.onerror = function(){ updateConnectionStatus(false); }
        } catch(e){ updateConnectionStatus(false); setTimeout(socket_init,3000); }
    }

    function updateConnectionStatus(connected){
        let el = document.getElementById('connectionStatus');
        if(connected){ el.textContent='已连接'; el.className='connection-status status-connected'; }
        else{ el.textContent='未连接'; el.className='connection-status status-disconnected'; }
    }

    // ===== 发送二进制数据 =====
    function send_packet(id, value){
        if(socket && socket.readyState===1){
            let valInt = Math.round(value); // 已放大 100 倍
            let buf = new Uint8Array(6);
            buf[0] = 0xFF; buf[1] = 0xAA;
            buf[2] = id;
            buf[3] = valInt & 0xFF;
            buf[4] = (valInt >> 8) & 0xFF;
            buf[5] = (buf[2]+buf[3]+buf[4]) & 0xFF;
            socket.send(buf.buffer);
        }
    }

    // ===== 摇杆类 =====
    function JoyStick(container){
        let obj = document.getElementById(container);
        let canvas = document.createElement("canvas");
        canvas.width = obj.clientWidth; canvas.height = obj.clientHeight;
        obj.appendChild(canvas);
        let ctx = canvas.getContext("2d");

        let centerX = canvas.width/2, centerY = canvas.height/2;
        let radius = (canvas.width - ((canvas.width/2)+10))/2;
        let maxMove = radius+5;
        let movedX=centerX, movedY=centerY;
        let autoReturn=true;

        let lastX=0, lastY=0, lastSendTime=0;

        function draw(){
            ctx.clearRect(0,0,canvas.width,canvas.height);
            ctx.beginPath(); ctx.arc(centerX,centerY,radius+30,0,2*Math.PI);
            ctx.lineWidth=2; ctx.strokeStyle="#4a90e2"; ctx.stroke();
            ctx.beginPath(); ctx.arc(movedX,movedY,radius,0,2*Math.PI);
            let grd=ctx.createRadialGradient(centerX,centerY,5,centerX,centerY,200);
            grd.addColorStop(0,"#4a90e2"); grd.addColorStop(1,"#4a90e2");
            ctx.fillStyle=grd; ctx.fill();
            ctx.lineWidth=2; ctx.strokeStyle="#4a90e2"; ctx.stroke();
        }

        function sendXY(){
            let now = Date.now();
            if(now - lastSendTime < 20) return;
            lastSendTime = now;

            let dx=(movedX-centerX)/maxMove;
            let dy=-(movedY-centerY)/maxMove;
            dx = Math.max(-1,Math.min(1,dx));
            dy = Math.max(-1,Math.min(1,dy));

            let xVal, yVal;
            if(container==='joy1Div'){
                xVal = Math.round(dx*100); yVal = Math.round(dy*100);
                if(Math.abs(xVal-lastX)<=5 && Math.abs(yVal-lastY)<=5) return;
                lastX = xVal; lastY = yVal;
                document.getElementById('joy1XValue').textContent=xVal;
                document.getElementById('joy1YValue').textContent=yVal;
                send_packet(0x01, yVal*100);
                send_packet(0x02, xVal*100);
            } else {
                let rollVal = Math.round(dx*30);
                let heightVal = Math.round((dy+1)*30);
                rollVal = Math.max(-30, Math.min(30, rollVal));
                heightVal = Math.max(0, Math.min(60, heightVal));
                if(Math.abs(rollVal-lastX)<=5 && Math.abs(heightVal-lastY)<=5) return;
                lastX = rollVal; lastY = heightVal;
                document.getElementById('joy2XValue').textContent=rollVal;
                document.getElementById('joy2YValue').textContent=heightVal;
                send_packet(0x03, rollVal*100);
                send_packet(0x04, heightVal*100);
            }
        }

        function updatePos(clientX,clientY){
            let dx = clientX - canvas.getBoundingClientRect().left - centerX;
            let dy = clientY - canvas.getBoundingClientRect().top - centerY;
            let dist = Math.sqrt(dx*dx + dy*dy);
            if(dist>maxMove){ let scale=maxMove/dist; dx*=scale; dy*=scale; }
            movedX = centerX+dx; movedY = centerY+dy;
            draw(); sendXY();
        }

        function release(){
            if(autoReturn){ 
                movedX=centerX; movedY=centerY; draw();
                lastX=0; lastY=0;
                if(container==='joy1Div'){
                    document.getElementById('joy1XValue').textContent='0';
                    document.getElementById('joy1YValue').textContent='0';
                    send_packet(0x01,0); send_packet(0x02,0);
                } else {
                    document.getElementById('joy2XValue').textContent='0';
                    document.getElementById('joy2YValue').textContent='30';
                    send_packet(0x03,0); send_packet(0x04,3000);
                }
            }
        }

        canvas.addEventListener("mousedown", e=>updatePos(e.clientX,e.clientY));
        canvas.addEventListener("mousemove", e=>{ if(e.buttons>0) updatePos(e.clientX,e.clientY); });
        canvas.addEventListener("mouseup", e=>release());
        canvas.addEventListener("touchstart", e=>updatePos(e.touches[0].pageX,e.touches[0].pageY));
        canvas.addEventListener("touchmove", e=>{ e.preventDefault(); updatePos(e.touches[0].pageX,e.touches[0].pageY); });
        canvas.addEventListener("touchend", e=>release());

        draw();
    }

    // ===== 初始化摇杆 =====
    new JoyStick('joy1Div');
    new JoyStick('joy2Div');

    // ===== Jump 按钮绑定 =====
    let jumpBtn = document.getElementById('jump');
    jumpBtn.addEventListener('mousedown', () => send_packet(0x10,100));
    jumpBtn.addEventListener('mouseup', () => send_packet(0x10,0));
    jumpBtn.addEventListener('touchstart', (e)=>{ e.preventDefault(); send_packet(0x10,100); });
    jumpBtn.addEventListener('touchend', (e)=>{ e.preventDefault(); send_packet(0x10,0); });

    </script>

)=====";

/**
 * @brief 解析从网页端收到的控制数据包
 * 
 * @param ucData 指向接收到的6字节数据
 * 
 * 数据协议（固定 6 字节）：
 *  Byte0: 0xFF
 *  Byte1: 0xAA
 *  Byte2: 控制 ID
 *  Byte3: 数据低 8 位
 *  Byte4: 数据高 8 位
 *  Byte5: 校验和（Byte2 + Byte3 + Byte4）
 */
static void WebSiteServer_ParseRxData(unsigned char *ucData)
{
    unsigned char ucSum = ucData[2] + ucData[3] + ucData[4];
    if(ucData[0] != 0xFF || ucData[1] != 0xAA || ucSum != ucData[5]){return;}       // 检查包头与校验位

    short sData = (short)(((unsigned short)ucData[4] << 8) | ucData[3]);       // 注意：上位机发送时放大了 100 倍

    switch(ucData[2])
    {
        case 0x01:      // 前后
            sReg[JoyY] = sData;
            break;

        case 0x02:      // 左右
            sReg[JoyX] = sData;
            break;

        case 0x03:      // 倾角
            sReg[TiltAngle] = sData;
            break;

        case 0x04:      // 高度
            sReg[RobHeight] = sData;
            break;

        case 0x10:      // 跳跃 1按下 0松开
            if(sData / 100 == 1){sReg[RobJump] = 1;}
            else{sReg[RobJump] = 0;}
            break;

        case 0xFA:
            if(sData / 100 == 1){sReg[SendFlag] = 1;}
            else{sReg[SendFlag] = 0;}
    }
}

/**
 * @brief 构建发送给网页端的数据包
 * 
 * @param uiLen 输出，数据包长度
 * @return unsigned char* 指向要发送的数据缓冲区
 */
static unsigned char *WebSiteServer_ParseTxData(unsigned int *uiLen)
{
    static unsigned char ucTxData[256];
    unsigned char ucIndex = 0;

    ucTxData[ucIndex++] = 0xFF;
    ucTxData[ucIndex++] = 0xAA;

    ucIndex++;      // 包长，先占位

    // // Data 1
    // ucTxData[ucIndex++] = 0x01;
    // memcpy(&ucTxData[ucIndex], &sReg[LQR_Distance], sizeof(float));
    // ucIndex += sizeof(float);

    // // Data 2
    // ucTxData[ucIndex++] = 0x02;
    // memcpy(&ucTxData[ucIndex], &sReg[LQR_Speed], sizeof(float));
    // ucIndex += sizeof(float);

    // // Data 3
    // ucTxData[ucIndex++] = 0x03;
    // memcpy(&ucTxData[ucIndex], &sReg[Height], sizeof(float));
    // ucIndex += sizeof(float);

    // // Data 4
    // ucTxData[ucIndex++] = 0x04;
    // memcpy(&ucTxData[ucIndex], &sReg[LQR_u], sizeof(float));
    // ucIndex += sizeof(float);

    // // Data 5
    // ucTxData[ucIndex++] = 0x05;
    // memcpy(&ucTxData[ucIndex], &sReg[ZeroPoint], sizeof(float));
    // ucIndex += sizeof(float);

    unsigned char ucSum = 0;
    for(unsigned char i = 0; i < ucIndex - 2; i++)
    {
        ucSum += ucTxData[i + 2];
    }
    ucTxData[ucIndex++] = ucSum;

    ucTxData[2] = ucIndex;      // 整包长度
    
    *uiLen = ucIndex;

    return ucTxData;
}

/**
 * @brief HTTP 根页面请求处理函数，分块发送
 * 
 * @param Req HTTP 请求句柄
 * @return esp_err_t ESP_OK 表示处理成功
 */
static esp_err_t WebSiteServer_RootHandler(httpd_req_t *Req)
{
    const char *cHtml = cBasicWebSite;
    const unsigned int uiChunkSize = 512;       // 每块 512 字节
    unsigned int uiLen = strlen(cHtml), uiOffset = 0;

    httpd_resp_set_type(Req, "text/html");      // 设置 Content-Type

    while(uiOffset < uiLen)
    {
        unsigned int uiSendLen = (uiLen - uiOffset) > uiChunkSize ? uiChunkSize : (uiLen - uiOffset);
        esp_err_t Err = httpd_resp_send_chunk(Req, cHtml + uiOffset, uiSendLen);
        if(Err != ESP_OK){return Err;}
        uiOffset += uiSendLen;
    }

    // 发送结束标记（长度为 0 的 chunk）
    return httpd_resp_send_chunk(Req, NULL, 0);
}

/**
 * @brief WebSocket 数据处理函数
 * 
 * @param Req WebSocket 请求句柄
 * @return esp_err_t ESP_OK 表示处理成功
 */
static esp_err_t WebSiteServer_Handler(httpd_req_t *Req)
{
    if(Req->method == HTTP_GET){return ESP_OK;}

    // 只保留最新的 WebSocket 连接
    static int iActiveWS_FD = -1;       // 当前有效 WebSocket 的 socket fd
    int iSocketFd = httpd_req_to_sockfd(Req);       // 获取当前请求的 socket fd
    if(iSocketFd < 0){return ESP_FAIL;}
    if(iActiveWS_FD >= 0 && iActiveWS_FD != iSocketFd)
    {
        httpd_sess_trigger_close(Webserver, iActiveWS_FD);
    }
    iActiveWS_FD = iSocketFd;       // 更新当前的 FD

    // 读取与解析数据
    httpd_ws_frame_t RxFrame = {0};
    esp_err_t Err = httpd_ws_recv_frame(Req, &RxFrame, 0);        // 第一次获取长度
    if(Err != ESP_OK){return Err;}
    if(RxFrame.type == HTTPD_WS_TYPE_BINARY && RxFrame.len == 6)
    {
        unsigned char ucPayload[RxFrame.len];
        RxFrame.payload = ucPayload;        // 将 ucPayload 的地址赋值给 RxFrame
        Err = httpd_ws_recv_frame(Req, &RxFrame, RxFrame.len);
        if(Err != ESP_OK){return Err;}      // 防止网络波动带来的接收错误
        WebSiteServer_ParseRxData(ucPayload);
    }

    // 发送数据
    if(sReg[SendFlag])
    {
        unsigned int uiTxLen;
        httpd_ws_frame_t TxFrame = {
            .type = HTTPD_WS_TYPE_BINARY,
            .payload = WebSiteServer_ParseTxData(&uiTxLen),
            .len = uiTxLen,
        };
        Err = httpd_ws_send_frame(Req, &TxFrame);
        if(Err != ESP_OK){return Err;}
    }

    return ESP_OK;
}

/**
 * @brief 初始化 WebServer 与 WebSocket
 * 
 * @note TCP 状态维护消耗巨大，不建议使用
 */
void WebSiteServer_Init(void)
{
    MyWiFi_Init(STA);       // 连接 WiFi

    httpd_config_t WebServerConfig = HTTPD_DEFAULT_CONFIG();
    WebServerConfig.core_id = 0;
    WebServerConfig.task_priority = 2;
    httpd_start(&Webserver, &WebServerConfig);

    httpd_uri_t RootUri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = WebSiteServer_RootHandler
    };
    httpd_register_uri_handler(Webserver, &RootUri);

    httpd_uri_t WsUri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = WebSiteServer_Handler,
        .is_websocket = 1
    };
    httpd_register_uri_handler(Webserver, &WsUri);
}
