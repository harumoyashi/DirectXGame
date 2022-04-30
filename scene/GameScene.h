#pragma once

#include "Audio.h"
#include "DebugText.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "Model.h"
#include "SafeDelete.h"
#include "Sprite.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include <DirectXMath.h>

/// <summary>
/// ゲームシーン
/// </summary>
class GameScene {

  public: // メンバ関数
	/// <summary>
	/// コンストクラタ
	/// </summary>
	GameScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// スピード設定
	/// x:現在いる座標,nx:次いく座標
	/// </summary>
	float SetSpeed(float x, float nx);

	/// <summary>
	/// インクリメントする変数のループ処理
	/// x:変数,loopBreak:ループする数(max)
	/// </summary>
	int NumLoop(int x,int loopBreak);

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

  private: // メンバ変数
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;
	DebugText* debugText_ = nullptr;

	/// <summary>
	/// ゲームシーン用
	/// </summary>

	//テクスチャハンドル
	uint32_t textureHandle_ = 0;

	// 3Dモデル
	Model* model_ = nullptr;

	//ワールドトランスフォーム
	WorldTransform worldTransform_[3];
	//ビュープロジェクション
	ViewProjection viewProjection_;

	//注視点番号
	int tViewNum = 0;
	int tNextNum = tViewNum + 1;

	//カメラ上方向の角度
	float viewAngle = 0.0f;

	//注視点移動開始
	bool isTargetChange = false;

	//注視点移動スピード
	float tSpeedX = 0.0f;
	float tSpeedY = 0.0f;

	//注視点が移動仕切るまでのフレーム数
	int tMoveFrame = 0;
};
