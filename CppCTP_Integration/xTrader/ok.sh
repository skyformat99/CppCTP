content=`head -n 4 Debug.h | tail -n 1`
echo $content
result=`echo $content | grep "//"`
echo $result

if [[ "$result" != "" ]]
then
        echo "Release Mode"
        mode=no_debug
        time=`date "+%Y-%m-%d_%H-%M-%S"`
        cmake . && make && cd bin && rm -f quant_ctp_XTrader_no_debug_* && mv quant_ctp_XTrader "quant_ctp_XTrader_${mode}_${time}"
else
        echo "Debug Mode"
        mode=debug
        time=`date "+%Y-%m-%d_%H-%M-%S"`
        cmake . && make && cd bin && rm -f quant_ctp_XTrader_debug_* && mv quant_ctp_XTrader "quant_ctp_XTrader_${mode}_${time}"
fi
echo "Finish Compile, Please check bin directory."
