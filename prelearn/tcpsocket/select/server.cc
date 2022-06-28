#include"server.hpp"
// int main()
// {
//     Server sv;
//     sv.Init();
//     sv.Start();
//     return 0;
// }

#include"sock.hpp"
#include"server.hpp"

int main(int argc,char* argv[])
{
  cout << sizeof(fd_set)*8 <<endl;
  if(argc != 2)
  {
    exit(1);
  }

  Server* svr = new Server(atoi(argv[1]));
  svr->Init();
  svr->Start();
  return 0;
}

