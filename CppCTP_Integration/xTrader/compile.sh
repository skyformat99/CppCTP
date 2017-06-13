if [[ "$1" = "debug" ]]
then
        echo "Debug Mode"
	sed -i '/#define DEBUG/s/.*#define.*DEBUG.*/#define DEBUG/' Debug.h
	mode=debug
        time=`date "+%Y-%m-%d_%H-%M-%S"`
        cmake . && make -j4 && cd bin && rm -f quant_ctp_XTrader_debug_* && mv quant_ctp_XTrader "quant_ctp_XTrader_${mode}_${time}"
	#echo "Finish Compile, Please check bin directory."
	echo -e "\033[31mFinish Compile ${mode} Version! Please Check bin folder.\033[0m"
elif [[ "$1" == "release" ]]
then
        echo "Release Mode"
	sed -i '/#define DEBUG/s/.*#define.*DEBUG.*/\/\/#define DEBUG/' Debug.h 
	mode=no_debug
        time=`date "+%Y-%m-%d_%H-%M-%S"`
        cmake . && make -j4 && cd bin && rm -f quant_ctp_XTrader_no_debug_* && mv quant_ctp_XTrader "quant_ctp_XTrader_${mode}_${time}"
	#echo "Finish Compile, Please check bin directory."
	echo -e "\033[31mFinish Compile ${mode} Version! Please Check bin folder.\033[0m"
else
	echo "Error Input"
fi
