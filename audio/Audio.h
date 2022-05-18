#pragma once

#include <array>
#include <cstdint>
#include <set>
#include <string>
#include <unordered_map>
#include <wrl.h>
#include <xaudio2.h>
#include "x3daudio.h"
#include "xapofx.h"
#include "mmreg.h"
#include "Input.h"

/// <summary>
/// オーディオ
/// </summary>
class Audio {
  public:
	// サウンドデータの最大数
	static const int kMaxSoundData = 256;

	// チャンクヘッダ
	struct ChunkHeader {
		char id[4];   // チャンク毎のID
		int32_t size; // チャンクサイズ
	};

	// RIFFヘッダチャンク
	struct RiffHeader {
		ChunkHeader chunk; // "RIFF"
		char type[4];      // "WAVE"
	};

	// FMTチャンク
	struct FormatChunk {
		ChunkHeader chunk; // "fmt "
		WAVEFORMATEX fmt;  // 波形フォーマット
	};

	// 音声データ
	struct SoundData {
		// 波形フォーマット
		WAVEFORMATEX wfex;
		// バッファの先頭アドレス
		BYTE* pBuffer;
		// バッファのサイズ
		unsigned int bufferSize;
		// 名前
		std::string name_;
	};

	// 再生データ
	struct Voice {
		uint32_t handle = 0u;
		IXAudio2SourceVoice* sourceVoice = nullptr;
	};

	//聞こえる方向や位置の変数
	X3DAUDIO_VECTOR EOrientFront = {0,1,0};
	X3DAUDIO_VECTOR EOrientTop   = {0,0,-1};
	X3DAUDIO_VECTOR EPosition    = {0,0,0};
	X3DAUDIO_VECTOR EVelocity    = {};
	X3DAUDIO_VECTOR LOrientFront = {0,-1,0};
	X3DAUDIO_VECTOR LOrientTop   = {0,0,1};
	X3DAUDIO_VECTOR LPosition    = {200.0f,0,0};
	X3DAUDIO_VECTOR LVelocity    = {};

	X3DAUDIO_HANDLE X3DInstance;

	//オーディオデータを扱う便利なインターフェース
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	//サブミックスボイス
	IXAudio2SubmixVoice* pSubmixVoice;
	IXAudio2MasteringVoice* masterVoice;

	//エミッタ構造体
	X3DAUDIO_LISTENER Listener = {};
	X3DAUDIO_EMITTER Emitter = {};

	// X3DAUDIO_DSP_SETTING構造体
	X3DAUDIO_DSP_SETTINGS DSPSettings = {0};

	XAUDIO2_VOICE_DETAILS deviceDetails;
	X3DAUDIO_CONE emitterCone;

	//エフェクト
	IUnknown* pXAPO;
	//XAUDIO2_EFFECT_DESCRIPTOR構造体
	XAUDIO2_EFFECT_DESCRIPTOR descriptor;
	//XAUDIO2_EFFECT_CHAIN構造体
	XAUDIO2_EFFECT_CHAIN chain;
	//リバーブパラメータ
	FXREVERB_PARAMETERS XAPOParameters;

	FormatChunk format = {};

	//X3DAudio values
	X3DAUDIO_DISTANCE_CURVE_POINT volumePoints[10];
	X3DAUDIO_DISTANCE_CURVE volumeCurve;

	X3DAUDIO_DISTANCE_CURVE_POINT reverbPoints[10];
	X3DAUDIO_DISTANCE_CURVE reverbCurve;

	/*std::unique_ptr<float[]> matrix;*/
	int reverbIndex = 0;

	/// <summary>
	/// オーディオコールバック
	/// </summary>
	class XAudio2VoiceCallback : public IXAudio2VoiceCallback {
	  public:
		// ボイス処理パスの開始時
		STDMETHOD_(void, OnVoiceProcessingPassStart)(THIS_ UINT32 BytesRequired){};
		// ボイス処理パスの終了時
		STDMETHOD_(void, OnVoiceProcessingPassEnd)(THIS){};
		// バッファストリームの再生が終了した時
		STDMETHOD_(void, OnStreamEnd)(THIS){};
		// バッファの使用開始時
		STDMETHOD_(void, OnBufferStart)(THIS_ void* pBufferContext){};
		// バッファの末尾に達した時
		STDMETHOD_(void, OnBufferEnd)(THIS_ void* pBufferContext);
		// 再生がループ位置に達した時
		STDMETHOD_(void, OnLoopEnd)(THIS_ void* pBufferContext){};
		// ボイスの実行エラー時
		STDMETHOD_(void, OnVoiceError)(THIS_ void* pBufferContext, HRESULT Error){};
	};

	static Audio* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const std::string& directoryPath = "Resources/");

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// WAV音声読み込み
	/// </summary>
	/// <param name="filename">WAVファイル名</param>
	/// <returns>サウンドデータハンドル</returns>
	uint32_t LoadWave(const std::string& filename);

	/// <summary>
	/// サウンドデータの解放
	/// </summary>
	/// <param name="soundData">サウンドデータ</param>
	void Unload(SoundData* soundData);

	void SetReverb(int reverbIndex);

	/// <summary>
	/// 音声再生
	/// </summary>
	/// <param name="soundDataHandle">サウンドデータハンドル</param>
	/// <param name="loopFlag">ループ再生フラグ</param>
	/// <param name="volume">ボリューム
	/// 0で無音、1がデフォルト音量。あまり大きくしすぎると音割れする</param>
	/// <returns>再生ハンドル</returns>
	uint32_t PlayWave(uint32_t soundDataHandle, bool loopFlag = false, float volume = 0.5f);

	/// <summary>
	/// 音声停止
	/// </summary>
	/// <param name="voiceHandle">再生ハンドル</param>
	void StopWave(uint32_t voiceHandle);

	/// <summary>
	/// 音声再生中かどうか
	/// </summary>
	/// <param name="voiceHandle">再生ハンドル</param>
	/// <returns>音声再生中かどうか</returns>
	bool IsPlaying(uint32_t voiceHandle);

	/// <summary>
	/// 音量設定
	/// </summary>
	/// <param name="voiceHandle">再生ハンドル</param>
	/// <param name="volume">ボリューム
	/// 0で無音、1がデフォルト音量。あまり大きくしすぎると音割れする</param>
	void SetVolume(uint32_t voiceHandle, float volume);

  private:
	Audio() = default;
	~Audio() = default;
	Audio(const Audio&) = delete;
	const Audio& operator=(const Audio&) = delete;

	Input* input_ = nullptr;

	// XAudio2のインスタンス
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	// サウンドデータコンテナ
	std::array<SoundData, kMaxSoundData> soundDatas_;
	// 再生中データコンテナ
	// std::unordered_map<uint32_t, IXAudio2SourceVoice*> voices_;
	std::set<Voice*> voices_;
	// サウンド格納ディレクトリ
	std::string directoryPath_;
	// 次に使うサウンドデータの番号
	uint32_t indexSoundData_ = 0u;
	// 次に使う再生中データの番号
	uint32_t indexVoice_ = 0u;
	// オーディオコールバック
	XAudio2VoiceCallback voiceCallback_;
};
