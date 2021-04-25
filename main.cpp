#include <iostream>
#include <ctime>
#include <cmath>
#include <random>
#include <queue>
#include <fstream>
#include <iomanip>
#include <unordered_map>

using namespace std;

double arrival_rate,
        avg_service_time;
int num_cpu;
bool multi_queue;

double invExpCDF (double);

class process
{
public:
    double service_time = -1,
            arrival_time = -1,
            departure_time = -1;
    int process_id = -1;

    process()
    {
        do
        {
            service_time = invExpCDF(1.0 / avg_service_time);
        } while (isinf(static_cast<int>(service_time)));
    }

    process(int id, double a_time)
    {
        do
        {

            service_time = invExpCDF(1.0 / avg_service_time);
        } while (isinf(static_cast<int>(service_time)));
        process_id = id;
        arrival_time = a_time;
    }
};

class event
{

public:
    bool arrival = false;
    bool departure = false;

    double time;

    process e_process;

    bool operator < (const event &e) const
    {
        return (this->time > e.time);
    }
    event(int pol, double t, process p)
    {
        if (pol == 1)
        {
            arrival = true;
            departure = false;
        }
        else
        {
            arrival = false;
            departure = true;
        }
        e_process = p;
        time = t;
    }
};

class CPU
{
public:
    queue<process> ready_que;
    priority_queue<event> local_event_queue;
    int total_ready = 0,
        processes = 0,
        total_processes = 0;
    double time = 0,
        idle = 0,
        next_availiable = 0,
        in_use = 0;
    
};

void populateEvents()
{

}

void outputData ()
{

}

int main(int argc, char* argv[])
{
    // fill list of arguments
    arrival_rate = atof(argv[1]);
    avg_service_time = atof(argv[2]);
    num_cpu = atoi(argv[3]);
    multi_queue = atoi(argv[4]);

    srand(time(NULL));

    if (argc != 5)
    {
        printf("program needs 5 (4) arguments received %d", argc);
        return 1;
    }

    ofstream fout;
    fout.open("table.txt");
    if (!fout)
    {
        cout << "Error opening table";
        return 2;
    }

    ofstream exlout;
    exlout.open("out.csv", ios_base::app);
    if (!exlout)
    {
        cout << "Error opening csv";
        return 3;
    }

    priority_queue<event> static_event_queue,
            mutable_seq;
    queue<process> ready_queue;
    vector<CPU> cpu_list;
    for (int i = 0; i < num_cpu; i++)
        cpu_list.push_back(CPU());

    double TbTarrivals,
            cur_time = 0,
            ttsum = 0,
            trn_time = 0,
            delta = 0,
            active_time = 0,
            utilization,
            throughput,
            avg_ready,
            avg_turnaround_time;
    int     processes = 1,
            total_ready = 0,
            NUM_PROCESSES = 20000;

    // Fills a table of arrival times and service times
    while (processes <= NUM_PROCESSES)
    {
        if (multi_queue)
        {
            double rand_index = rand() % num_cpu;

            TbTarrivals = invExpCDF(arrival_rate);
            event temp = event(1, cur_time, process(processes, cur_time));
            cpu_list[rand_index].local_event_queue.push(temp);
            cpu_list[rand_index].processes++;
            // make output step easier
            static_event_queue.push(temp);
        }
        else
        {
            // Creates Events and pushes them into the global event queue

            TbTarrivals = invExpCDF(arrival_rate);
            event temp = event(1, cur_time, process(processes, cur_time));
            static_event_queue.push(temp);
        }
        cur_time += TbTarrivals;
        processes++;
    }

    mutable_seq = static_event_queue; // create a equeue that we can change by copying the original

    cur_time = 0; // reset time
    if (multi_queue)
        for (auto& processor: cpu_list)
        {
            while (!processor.local_event_queue.empty())
            {
                // if an event arrives while we are handling another, add it to the queue
                while (processor.time >= processor.local_event_queue.top().time) {
                    if (processor.local_event_queue.empty())
                        break;

                    processor.ready_que.push(processor.local_event_queue.top().e_process);
                    processor.local_event_queue.pop();
                    processor.processes--;
                }

                // empty the ready queues, log departures, increment time\
                s
                while (!processor.ready_que.empty())
                {
                    processor.total_ready += processor.ready_que.size();
                    processor.time += processor.ready_que.front().service_time;
                    processor.in_use += processor.ready_que.front().service_time;
                    processor.ready_que.front().departure_time = processor.time;
                    // ************************************************************************* revist below
                    static_event_queue.push(event(0, processor.time, processor.ready_que.front()));
                    processor.ready_que.pop();

                }

                // if we have to wait for more events to come the cpu is idle; increment time and move to next event
                if (processor.local_event_queue.top().time > processor.time) {
                    if (processor.local_event_queue.empty())
                        break;
                    //cout << "idle " << delta << " " << processor.local_event_queue.top().time << endl;
                    delta = processor.local_event_queue.top().time - processor.time;
                    processor.idle += delta;
                    processor.time += delta;
                }

            }
        }
    else
    {
        while (!mutable_seq.empty())
        {
            // if an event arrives while we are handling another, add it to the queue
            while (cur_time >= mutable_seq.top().time) {
                if (mutable_seq.empty())
                    break;

                ready_queue.push(mutable_seq.top().e_process);
                mutable_seq.pop();
            }

            // empty the ready queues, log departures, increment time

            while (!ready_queue.empty())
            {
                // Code used to find which CPU is most ready
                int next_cpu_index = 0;
                for (int i = 0; i < num_cpu; i++)
                    if (cpu_list[i].next_availiable < cpu_list[next_cpu_index].next_availiable)
                        next_cpu_index = i;


                // If no CPU is currently avaliable, stall until we find one that is availiable.
                if (cpu_list[next_cpu_index].next_availiable > cur_time)
                    cur_time = cpu_list[next_cpu_index].next_availiable;

                // Used for calculating the average ready queue size data inside at present is not important,
                // it will be later divided by NUM_PROCESSES to determine aver ready queue size.
                total_ready += ready_queue.size();

                // Departure time is simple the current time + service time
                // Time taken until we use the CPU is
                ready_queue.front().departure_time = cur_time + ready_queue.front().service_time;
                cpu_list[next_cpu_index].idle += (cur_time - cpu_list[next_cpu_index].next_availiable);
                cpu_list[next_cpu_index].in_use += ready_queue.front().service_time;
                cpu_list[next_cpu_index].next_availiable = cur_time + ready_queue.front().service_time;


                // ************************************************************************* revist below
                static_event_queue.push(event(0, cur_time, ready_queue.front()));
                ready_queue.pop();
            }

            // if we have to wait for more events to come the cpu is idle; increment time and move to next event
            if (mutable_seq.top().time > cur_time) {
                if (mutable_seq.empty())
                    break;
                //cout << "idle " << delta << " " << mutable_seq.top().time << endl;
                delta = mutable_seq.top().time - cur_time;
                //cpu_idle_time += delta;
                cur_time += delta;

            }
        }

    }


    for (auto processor : cpu_list)
        active_time += processor.in_use;
    active_time /= num_cpu;


    if (multi_queue)
    {
        cur_time = 0;
        for (auto processor : cpu_list)
            if (processor.time > cur_time)
                cur_time = processor.time;

        // to get average active time, sum all active times  which we already did then divide by number of cpus
        utilization = active_time / cur_time;
        total_ready = 0;
        for (auto processor : cpu_list)
        {
            total_ready += processor.total_ready;
        }
    }

    //exlout << "Lambda,Service Time,Number of CPUs,Multi-queue Enabled,Throughput,Avg. Ready Queue Size,Turnaround Time,Utilization;
    fout << right << "ID" << setw(20) << "Arrival Time" << setw(20) << "Departure Time" << setw(20)
         << "Turnaround Time" << endl;

    while (!static_event_queue.empty())
    {
        if (static_event_queue.top().departure)
        {
            trn_time = static_event_queue.top().e_process.departure_time -
                       static_event_queue.top().e_process.arrival_time;
            ttsum += trn_time;

            if (static_event_queue.top().e_process.process_id % 500 == 0)
            {
                fout << static_event_queue.top().e_process.process_id << setw(20)
                     << static_event_queue.top().e_process.arrival_time
                     << setw(20)
                     << static_event_queue.top().e_process.departure_time << setw(20) << trn_time << endl;
            }
        }
        static_event_queue.pop();
    }

    utilization = active_time / cur_time;
    avg_turnaround_time = ttsum / NUM_PROCESSES;
    throughput = NUM_PROCESSES / cur_time;
    avg_ready = total_ready / static_cast<double>(NUM_PROCESSES);

    fout << endl
         << "Avg Turnaround time: " << avg_turnaround_time << endl
         << "CPU Utilization: " << utilization << endl
         << "Throughput: " << throughput << " Processes per second" << endl
         << "Avg ready: " << avg_ready << endl;

    exlout << endl
           << argv[1] << argv[2] << argv[3] << argv[4] << ',' << throughput << ',' << avg_ready << ','
           << avg_turnaround_time << ',' << utilization;

    fout.close();
    return 0;
}

double invExpCDF ( double lmda)
{
    double prob;

    do
    {
        prob = rand() / static_cast<double>(RAND_MAX);
    } while (prob == 1 || prob == 0);

    return ( (-1 * log( 1 - prob) )/ lmda);
}