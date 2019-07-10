#include "Process.h"
#include <vector>
//这就是一个容器类型的东西，你可以在这个ProcessContainer中放置最新一段时间的process的内容
//而这个process的内容，是可以随着时间变化的，我一般需要最新的process信息
//比如我要一个篮子里放西红柿，我总是希望篮子里放新鲜的西红柿，而不是不能用的西红柿
//所以你可以参考这个模型，建立一个你的股票数据的container, 不断得按照一定要求更新这个container的数据，从而算出这个类的你需要的指标

//当然了这个容器中需要有一个对象，比如西红柿，或者例如这里的Process，你提供途径来更新容器中的Process们
class ProcessContainer{

private:
    std::vector<Process*> _list;
public:
    ProcessContainer(){
        this->refreshList();
    }
    ~ProcessContainer(){
        for (auto proc : _list) {
            delete proc;
        }
    }
    void refreshList();
    std::string printList();
    std::vector<std::vector<std::string>> getList();
};

void ProcessContainer::refreshList(){
    std::cout << "begin" << std::endl;
    std::vector<std::string> pidList = ProcessParser::getPidList();
    std::cout << "end" << std::endl;
    std::cout << "pidList size:" << pidList.size() <<std::endl;
    this->_list.clear();
    for(auto pid : pidList){
        Process * proc = new Process(pid);
        this->_list.push_back(proc);
    }
}
std::string ProcessContainer::printList(){
    std::string result="";
    for(int i=0;i<this->_list.size();i++){
        result += this->_list[i]->getProcess();
    }
    return result;
}
std::vector<std::vector<std::string> > ProcessContainer::getList(){
    std::vector<std::vector<std::string>> values;
    std::vector<std::string> stringifiedList;
    for(int i=0; i<ProcessContainer::_list.size(); i++){
        stringifiedList.push_back(ProcessContainer::_list[i]->getProcess());
    }
    int lastIndex = 0;
    for (int i=0; i<stringifiedList.size();i++){
        if(i %10 == 0 && i > 0){
          std::vector<std::string>  sub(&stringifiedList[i-10], &stringifiedList[i]);
          values.push_back(sub);
          lastIndex = i;
        }
        if(i == (ProcessContainer::_list.size() - 1) && (i-lastIndex)<10){
            std::vector<std::string> sub(&stringifiedList[lastIndex],&stringifiedList[i+1]);
            values.push_back(sub);
        }
   }
    return values;
}
