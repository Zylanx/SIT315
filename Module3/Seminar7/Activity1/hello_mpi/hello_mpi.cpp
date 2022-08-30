#include <mpi.h>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;


// Constants
constexpr int MASTER = 0;  // The rank of the master process

constexpr int MESSAGE_SIZE = 13;  // The size of the message to be passed, including the null terminator
constexpr char MESSAGE[MESSAGE_SIZE] = "Hello World!";


int main(int argc, char** argv) {
    int numtasks, rank, name_len, tag = 1;
    char name[MPI_MAX_PROCESSOR_NAME];
    char message_buffer[MESSAGE_SIZE];  // Buffer to store messages to send/receive

    // Initialize the MPI environment
    MPI::Init(argc, argv);

    // Get the number of tasks/process
    numtasks = MPI::COMM_WORLD.Get_size();

    // Get the rank
    rank = MPI::COMM_WORLD.Get_rank();

    // Find the processor name
    MPI::Get_processor_name(name, name_len);


    // === Send and Receive Methods ===

    // Switch operation based on master vs client ranks.
    switch(rank)
    {
        case MASTER:
        {  // Brackets are just to show the cases better
            // Copy the message into the buffer so it can be sent from the master
            strcpy(message_buffer, MESSAGE);

            cout << "=== Send and Receive Method ===" << endl;

            // Loop through the tasks, skipping the master task, sending the message to each one
            for (auto dest = MASTER + 1; dest < numtasks; dest++)
            {
                MPI::COMM_WORLD.Send(message_buffer, MESSAGE_SIZE, MPI::BYTE, dest, 0);
            }

            break;
        }
        default:  // Client processes
        {
            // Receive the message, then print it out.
            MPI::COMM_WORLD.Recv(message_buffer, MESSAGE_SIZE, MPI::BYTE, MASTER, MPI::ANY_TAG);

            cout << message_buffer << endl;

            // Clear out the array for the next method
            memset(message_buffer, 0, MESSAGE_SIZE);
            break;
        }
    }

    // Add a pause to force the OS to flush the output, it will not do it consistently otherwise.
    MPI::COMM_WORLD.Barrier();
    this_thread::sleep_for(chrono::milliseconds(250));
    MPI::COMM_WORLD.Barrier();

    // Broadcast and receive a message from the broadcast group rooted on the master.
    MPI::COMM_WORLD.Bcast(message_buffer, MESSAGE_SIZE, MPI::BYTE, MASTER);

    // // Switch final output operation based on master vs client ranks.
    switch(rank)
    {
        case MASTER:
        {
            cout << flush << "=== Broadcast Method ===" << endl;

            // Add a pause to force the OS to flush the output, again
            this_thread::sleep_for(chrono::milliseconds(250));
            MPI::COMM_WORLD.Barrier();

            break;
        }
        default:  // Client processes
        {
            MPI::COMM_WORLD.Barrier();

            cout << message_buffer << endl;
            break;
        }
    }

    // Finalize the MPI environment
    MPI_Finalize();
}
