#-*- coding: utf-8 -*-
from ctypes import *
# 非同期用のコールバック関数定義 
CBFunc = CFUNCTYPE(None, POINTER(c_ubyte))

# 非同期用のコールバック関数
def cbfunc(response):
	print 'py call back called'
	for i in range(20):
		print '%x' % response[i]
	# Felicaポーリング停止
	ret = asi.StopFelicaPolling(pBuf)
	# ASI後始末
	ret = asi.DisposeASI()

def main():
    # 受信バッファ
	global pBuf
	pBuf = create_string_buffer(50)
    	# ライブラリ呼び出し
	global asi
	asi = cdll.LoadLibrary("libasi.so")
   # ASI初期化
	ret = asi.InitASI(2)
   # ROMバージョン取得
	#ret = asi.GetRomVersion(pBuf)

	# Felicaモードセット
	ret = asi.SetFelicaMode(pBuf)
	'''
	######################
	# 同期Felicaポーリング開始
	ret = asi.StartFelicaPollingSync(1, pBuf)
	print repr(pBuf.raw)
	# Felicaポーリング停止
	ret = asi.StopFelicaPolling(pBuf)
	# ASI後始末
	ret = asi.DisposeASI()
	'''
	
	######################
    	# コールバック関数ポインタ生成
	pFunc = CBFunc(cbfunc)
    	# 非同期Felicaポーリング開始
	ret = asi.StartFelicaPollingAsync(1, pBuf, pFunc)
    
	while True:
		pass
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
if __name__ == "__main__":
    main()  