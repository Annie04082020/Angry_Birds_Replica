# Abstract

遊戲名稱：Angry Birds

組員：
- 111310452 黃安華
- 113590039 許兆雲 

# Game Introduction
Angry Birds 是一款 2D 多關卡益智遊戲，玩家透過彈弓發射不同種類的 Angry Bird 來攻擊豬的防禦堡壘。豬會用木頭、石頭、冰塊等不同材料建造防禦，而不同種類的 Angry Bird 也各有獨特的異能和特效，玩家需要策略性地選擇使用。

[遊戲連結](https://www.angrybirds.com/)
[遊玩畫面](https://www.youtube.com/watch?v=tmcG-vbI6DQ)

# Development timeline
第一階段：基礎框架與素材準備 (Week 01 - 04)
- Week 01：和平紀念日放假
- Week 02：撰寫 Proposal、完成練習、蒐集素材
 ```每週一、四晚上固定確認開發進度；週五課程時間開發，晚上確認白天 meeting 找到的問題```  
  - [X] 訂定組內進度確認時間
  - [X] 111310452 PTSD Giraffe 通關
  - [X] 113590039 PTSD Giraffe 通關
  - [X] 撰寫 Proposal
  - [X] PTSD 環境建置(Repo Clone)
  - [X] 蒐集素材(pc版exe)
Week 03：發射邏輯與資源管理系統
  - [X] 安華：建立開發時程管理工具與講解 ([GitHub Project](https://github.com/users/Annie04082020/projects/3/views/1))
  - [ ] 安華：實作彈弓發射與拋物線路徑雛形
  - [ ] 安華：優化彈弓拖曳效果與動態路徑預測
  - [ ] 安華/兆雲：處理 PC 版素材，將原始資料拆解成 .png 並切割 Sprite
  - [ ] 兆雲：Git Learning Practice
  - [ ] 兆雲：實作 ResourceManager (負責 Singleton 載入所有圖片與音效素材)
- Week 04：初步碰撞偵測與地圖系統
  - [ ] 安華：實作簡單圓形 vs 矩形碰撞判定
  - [ ] 兆雲：實作背景渲染與地圖讀取器 (能從檔案讀取物件座標)

第二階段：核心系統與復刻第一關 (Week 05 - 08)
- Week 05：攝影機系統與 Level 1 復刻
  - [ ] 安華：實作 Camera Smooth Follow (追蹤小鳥飛行)
  - [ ] 兆雲：復刻原作 Level 1 佈局 (純木頭結構)
- Week 06：受力反應與物理細節
  - [ ] 安華：實作碰撞後的衝量傳遞 (讓木塊會倒塌、滑動)
  - [ ] 兆雲：實作物件受損的圖片切換 (血量減少對應不同破裂圖)
- Week 07：生命值判定與 Level 2-3 復刻
  - [ ] 安華：實作豬被撞擊/壓扁的傷害判定邏輯
  - [ ] 兆雲：復刻原作 Level 2 & 3 佈局 (加入石頭防禦結構)
- Week 08：期中整合測試
  - [ ] 全員：整合「發射 -> 碰撞 -> 倒塌 -> 消滅」完整 Loop
  - [ ] 全員：準備期中 Demo 報告

第三階段：功能擴展與進階關卡 (Week 09 - 13)
- Week 09：代碼重構與 Bug 修復
  - [ ] 全員：處理記憶體管理與物件重置 (Restart) 功能
- Week 10：特殊鳥種能力 (Inheritance)
  - [ ] 安華：實作黃鳥加速 (Speed Boost) 與藍鳥分裂邏輯
  - [ ] 兆雲：實作技能觸發特效與 復刻原作 Level 4-10 (分階段完成)
- Week 11：進階環境互動 (TNT)
  - [ ] 安華：實作 TNT 爆炸徑向衝擊波 (Radial Force)
  - [ ] 兆雲：實作 TNT 特效與環境物件佈置
- Week 12：繩子物理與特殊材質
  - [ ] 安華：嘗試實作簡易繩子與冰塊低摩擦力
  - [ ] 兆雲：加入不同材質碰撞音效與關卡細節
- Week 13：計分系統與三星判定
  - [ ] 安華：實作計分邏輯與 100% 復刻之星星判定
  - [ ] 兆雲：製作結算介面與關卡切換選單

第四階段：最後打磨與優化 (Week 14 - 17)
- Week 14-16：10 關完整測試與優化
  - [ ] 全員：進行 10 個關卡的流暢度測試與數值微調
  - [ ] 全員：加入背景音樂與原作風格之按鈕、轉場動畫
- Week 17：提交
  - [ ] 拍攝影片
  - [ ] 製作遊戲簡報
  - [ ] 驗收並提交

# OOP 修課證明 (if you need)

![111310452 OOP 修課證明](111310452_OOP.jpg)
![111310452 PTSD Giraffe 通關](111310452_PTSD_giraffe.png)