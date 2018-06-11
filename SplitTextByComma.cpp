#include <iostream> 
#include <string>
#include <sstream>
#include <vector>

using namespace std; 

std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

int main()
{ 
   string str = "ß,ä,ö,ü,Ä,Ö";
  std::vector<std::string> result;
  result = split(str,',');
  
  
  for(unsigned int i=0;i<result.size();i++)
    std::cout<<result[i]<<std::endl;
    
   return 0;
}
 
