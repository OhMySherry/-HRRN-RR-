#include <iostream>

#define MaxNum 100                                   // 最大进程数
double CurrentTime = 0.0;                            // 当前时间
double AverageWT_HRRN = 0.0, AverageWT_RR = 0.0;     // 平均等待时间
double AverageTAT_HRRN = 0.0, AverageTAT_RR = 0.0;   // 平均周转时间
double AverageWTAT_HRRN = 0.0, AverageWTAT_RR = 0.0; // 平均带权周转时间
int ProNum = 0;                                      // 进程数量
int DisplayNum = 0;                                  // 输出进程信息数量
int ProIndex = 0;                                    // 已完成的进程数量
int TimeSlice = 0;                                   // 时间片
using namespace std;

// 进程结构体
typedef struct PROCESS
{
    int index;                    // 进程序号
    int RunedTime;                // RR算法已服务的时间
    int Signal;                   // 第一次运行标志位 0表示该进程第一次运行 否则为1
    int ProStatus;                // 进程状态 0表示等待 1表示运行 2表示完成
    double ServiceTime;           // 服务时间
    double ArrivalTime;           // 到达时间
    double StartTime;             // 开始时间
    double FinishTime;            // 结束时间
    double TurnArroundTime;       // 周转时间
    double WaitTime;              // 等待时间
    double ResponseRatio;         // 响应比
    double WeightTurnArroundTime; // 带权周转时间
} PRO[MaxNum];

// 输出各进程的到达时间和服务时间
void display_base(PRO PC, int n)
{
    cout << endl;
    cout << "Process Num\t"
         << "Arrival Time\t"
         << "CPU Burst" << endl;
    for (int t = 0; t < n; t++)
        cout << PC[t].index << "\t\t" << PC[t].ArrivalTime << "\t\t" << PC[t].ServiceTime << endl;
}

// 模拟整个调度过程 输出每个时刻的进程运行状态
void display_status(PRO PC, int n)
{
    cout << endl;
    cout << "Process Num\t"
         << "Start Time\t"
         << "End Time\t"
         << "Ready Queue" << endl;
    for (int t = 0; t < n; t++) // 循环输出每个进程的服务时间内 处在等待队列的进程 即到达时间在当前进程开始和结束时间之间的进程
    {
        cout << PC[t].index << "\t\t" << PC[t].StartTime << "\t\t" << PC[t].FinishTime << "\t\t";
        for (int q = t + 1; q < n; q++)
        {
            if (PC[q].ArrivalTime <= PC[t].FinishTime)
                cout << PC[q].index << " ";
        }
        cout << endl;
    }
}

// 输出各进程的周转时间、带权周转时间和等待时间
void display_time(PRO PC, int n)
{
    cout << endl;
    cout << "Process Num\t"
         << "Turnaround Time\t"
         << "Weighted Turnaround Time\t"
         << "Waiting Time" << endl;
    for (int t = 0; t < n; t++)
        cout << PC[t].index << "\t\t" << PC[t].TurnArroundTime << "\t\t" << PC[t].WeightTurnArroundTime << "\t\t\t\t" << PC[t].WaitTime << endl;
}

// 输出所有进程的的平均周转时间、带权平均周转时间和平均等待时间
void display_average()
{
    cout << endl;
    cout << "Algorithm\t"
         << "Average TAT\t"
         << "Average WTAT\t"
         << "Average WT" << endl;
    cout << "HRRN\t\t" << AverageTAT_HRRN << "\t\t" << AverageWTAT_HRRN << "\t\t" << AverageWT_HRRN << endl;
    cout << "RR\t\t" << AverageTAT_RR << "\t\t" << AverageWTAT_RR << "\t\t" << AverageWT_RR << endl;
}

// 按到达时间排序
void SortArrival(PRO &PC, int n)
{
    PROCESS temp;
    for (int i = 0; i < n; i++)
    {
        int min = i;
        for (int j = i + 1; j < n; j++)
            if (PC[j].ArrivalTime < PC[min].ArrivalTime)
                min = j;
        temp = PC[i];
        PC[i] = PC[min];
        PC[min] = temp;
    }
}

// 计算各项时间
void CountTime(PRO &PC, int n)
{
    PC[0].StartTime = PC[0].ArrivalTime;
    PC[0].FinishTime = PC[0].ArrivalTime + PC[0].ServiceTime;
    PC[0].TurnArroundTime = PC[0].FinishTime - PC[0].ArrivalTime;
    PC[0].WaitTime = 0;
    PC[0].WeightTurnArroundTime = PC[0].TurnArroundTime / PC[0].ServiceTime;
    double sumWT = PC[0].WaitTime, sumTAT = PC[0].TurnArroundTime, sumWTAT = PC[0].WeightTurnArroundTime;
    for (int m = 1; m < n; m++)
    {
        if (PC[m].ArrivalTime >= PC[m - 1].FinishTime)
        {
            PC[m].StartTime = PC[m].ArrivalTime;
            PC[m].WaitTime = 0;
        }
        else
        {
            PC[m].StartTime = PC[m - 1].FinishTime;
            PC[m].WaitTime = PC[m - 1].FinishTime - PC[m].ArrivalTime;
        }
        PC[m].FinishTime = PC[m].StartTime + PC[m].ServiceTime;
        PC[m].TurnArroundTime = PC[m].FinishTime - PC[m].ArrivalTime;
        PC[m].WeightTurnArroundTime = PC[m].TurnArroundTime / PC[m].ServiceTime;
        sumWT += PC[m].WaitTime;
        sumTAT += PC[m].TurnArroundTime;
        sumWTAT += PC[m].WeightTurnArroundTime;
    }
    AverageWT_HRRN = sumWT / n;
    AverageTAT_HRRN = sumTAT / n;
    AverageWTAT_HRRN = sumWTAT / n;
}

// HRRN算法
void HRRN(PRO &PC, int n)
{
    SortArrival(PC, n); // 先按到达时间排序
    // 类比选择排序的思想 从第二个到达的进程开始 遍历当前进程后面的进程 如果有响应比比当前最短响应比更高的 则交换位置
    int End = PC[0].ArrivalTime + PC[0].ServiceTime; // 之前服务的结束时间
    int rmax = 1;                                    // 当前最高响应比的索引
    PROCESS temp;
    for (int x = 1; x < n; x++)
    {
        for (int r = x; r < n; r++)
        {
            if (PC[r].ArrivalTime <= End)
                PC[r].ResponseRatio = (End - PC[r].ArrivalTime + PC[r].ServiceTime) / PC[r].ServiceTime;
        }
        for (int y = x + 1; y < n; y++)
        {
            if (PC[y].ArrivalTime <= End && PC[y].ResponseRatio >= PC[rmax].ResponseRatio)
                rmax = y;
        }
        // 将当前进程与等待队列中响应比最大的进程进行交换
        temp = PC[x];
        PC[x] = PC[rmax];
        PC[rmax] = temp;
        End += PC[x].ServiceTime;
        rmax = x + 1; // 重置rmax为当前等待队列中的首位
    }
    CountTime(PC, n);      // 计算各项时间
    display_status(PC, n); // 模拟整个调度过程，输出每个时刻的进程运行状态
    display_time(PC, n);   // 输出各进程的周转时间、带权周转时间和等待时间
}

// RR输出函数
void PrintRR(PRO PC)
{
    cout << endl
         << "Current Time : " << CurrentTime << endl;
    cout << "Process Num\t"
         << "Start Time\t"
         << "Runed Time\t"
         << "Process Status" << endl;
    for (int t = 0; t < DisplayNum; t++) // 循环输出每个进程的服务时间内 处在等待队列的进程 即到达时间在当前进程开始和结束时间之间的进程
        cout << PC[t].index << "\t\t" << PC[t].StartTime << "\t\t" << PC[t].RunedTime << "\t\t" << PC[t].ProStatus << endl;
}

// 改变就绪队列的顺序 将当前进程移动到队尾
void MovePro(PRO &PC, int index)
{
    int Num = 0;
    for(int s = 0;s<ProNum;s++) // 更新等待队列的进程数量
        if(PC[s].ArrivalTime<=CurrentTime)
            Num++;
    PROCESS temp = PC[index];
    for (int i = index; i < Num - 1; i++)
    {
        PC[i] = PC[i + 1];
    }
    PC[Num - 1] = temp;
}

// 删除已完成的进程
void RemovePro(PRO &PC, int index)
{
    MovePro(PC, index);
    PC[ProNum - 1].ProStatus = 2;
    if (ProNum == 1) // 服务全部结束
    {
        cout << endl
             << "Finally Result : " << endl;
        PrintRR(PC);
    }
    ProIndex++;
    ProNum--;
}

// 判断一个进程是否完成并计算时间
int IfProEnd(PRO &PC, int index)
{
    if (PC[index].ServiceTime == PC[index].RunedTime)
    {
        PC[index].FinishTime = CurrentTime;
        PC[index].TurnArroundTime = PC[index].FinishTime - PC[index].ArrivalTime;
        PC[index].WeightTurnArroundTime = PC[index].TurnArroundTime / PC[index].ServiceTime;
        RemovePro(PC, index);
        return 1;
    }
    return -1;
}

// 指定时间片进程运行
int RunProcess(PRO &PC, int index)
{
    for (int i = 0; i < TimeSlice; i++) // 每个单位时间循环一次
    {
        CurrentTime++;
        if (PC[index].Signal == 0)
        {
            PC[index].StartTime = CurrentTime - 1;
            PC[index].Signal = 1;
        }
        PC[index].ProStatus = 1;
        PC[index].RunedTime++;
        PrintRR(PC); // 每个单位时间输出一次结果
        if (IfProEnd(PC, index) == 1)
            return 0;
    }
    MovePro(PC, index);
    PC[ProNum - 1].ProStatus = 0;
    return 0;
}

// RR算法
void RR(PRO &PC, int n)
{
    for (int s = 0; s < n; s++) // 标志位初始化为0
    {
        PC[s].Signal = 0;
        PC[s].ProStatus = 0;
        PC[s].RunedTime = 0;
    }
    cout << "TimeSlice = ";
    cin >> TimeSlice;
    SortArrival(PC, n); // 先按到达时间排序
    cout << endl;
    while (1)
    {
        if (CurrentTime >= PC[0].ArrivalTime)
        {
            RunProcess(PC, 0);
        }
        else
        {
            MovePro(PC, 0);
            PC[ProNum - 1].ProStatus = 0;
        }
        if (ProIndex == n)
            break; // 已完成的进程数量等于总数量 则退出循环
    }
    SortArrival(PC, n); // 按到达时间排序 使输出的结果更方便观看
    for (int i = 0; i < n; i++)
    {
        PC[i].FinishTime = PC[i].ArrivalTime + PC[i].TurnArroundTime;
        PC[i].WaitTime = PC[i].TurnArroundTime - PC[i].ServiceTime;
    }
    cout << endl
         << "Process Num\t"
         << "Start Time\t"
         << "End Time"
         << endl;
    for (int j = 0; j < n; j++)
    {
        cout << PC[j].index << "\t\t" << PC[j].StartTime << "\t\t" << PC[j].FinishTime << endl;
    }
    display_time(PC, DisplayNum); // 输出各进程的周转时间、带权周转时间和等待时间
    // 计算平均时间
    double sumWT = PC[0].WaitTime, sumTAT = PC[0].TurnArroundTime, sumWTAT = PC[0].WeightTurnArroundTime;
    for (int m = 1; m < n; m++)
    {
        sumWT += PC[m].WaitTime;
        sumTAT += PC[m].TurnArroundTime;
        sumWTAT += PC[m].WeightTurnArroundTime;
    }
    AverageWT_RR = sumWT / n;
    AverageTAT_RR = sumTAT / n;
    AverageWTAT_RR = sumWTAT / n;
}

int main()
{
    PRO pc;
    int n;
    cout << "请输入进程的数量：";
    cin >> n;
    DisplayNum = n;
    ProNum = n;
    cout << "请输入每个进程的到达时间和服务时间：" << endl;
    for (int i = 0; i < n; i++)
    {
        pc[i].index = i + 1;
        cin >> pc[i].ArrivalTime >> pc[i].ServiceTime;
    }
    display_base(pc, n);
    int choice;
    cout << endl
         << "输入0-输出平均时间并退出程序  输入1-HRRN  输入2-RR" << endl
         << "请选择要执行的操作：";
    cin >> choice;
    while (choice != 0)
    {
        if (choice == 1)
            HRRN(pc, n);
        if (choice == 2)
            RR(pc, n);
        cout << endl
             << "请输入接下来要执行的操作：";
        cin >> choice;
    }
    display_average();
    cout << endl;
    system("pause");
    return 0;
}