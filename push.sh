time=`date "+%Y-%m-%d_%H-%M-%S"`

git add .
git commit -m "${time} backup from linux"
git push origin CppCTP_API_ypf
echo "Finished Push!"