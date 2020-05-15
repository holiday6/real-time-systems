sudo ./simple_thread >> 5ms_evidant.txt &
sleep 10
sudo pkill -SIGKILL simple_thread
