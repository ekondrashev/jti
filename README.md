# Compile and Run on linux
`gcc -Wall -g -I$JAVA_HOME/include -I$JAVA_HOME/include/linux -otarget/libhd_on_fnfe_agent.so -lpthread -fPIC -shared src/main/c/hd_on_fnfe_agent.c && java -agentpath:./tart/libhd_on_fnfe_agent.so="Ljava/io/FileNotFoundException;" -cp ./target/classes ek.Main`

## Issues
If jmap is unable to attach to the process, try this
`echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope`