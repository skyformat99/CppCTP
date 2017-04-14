@echo off
:begin
echo ***********************
echo *确认编译? y or n
set input=
set /p input=*
if %input% == y (
	python setup.py build_ext --inplace
	echo ***********************
	echo.
	goto begin
) else (
	if %input% == n (
		echo ***********************
		echo "按任意键退出"
		pause
	) else (
		echo ***********************
		echo "输入错误!重新运行"
		pause
	)
)