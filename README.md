TaxiServiceOS

    Practical Assignment - Operating Systems (2025/2026)

    Simulation of an autonomous taxi fleet management platform in a UNIX environment.

Project Description

This project consists of a platform composed of three main processes (Cliente, Controlador, Veiculo) that communicate with each other using Inter-Process Communication (IPC) mechanisms provided by the Linux/UNIX operating system.

The goal is to simulate a transport service scheduling system, where clients request trips and a central controller manages a fleet of autonomous vehicles, assigning them to requests and monitoring their progress in real-time.
Features
1. Client Application (Cliente)

    Command Interface: Allows the user to schedule (agendar), consult (consultar), cancel (cancelar) services, and exit (sair) the application.

    Multithreading:

        Dedicated thread for sending commands via user input.

        Dedicated thread for asynchronous reception of server notifications (e.g., "Taxi arrived").

    State Validation: Prevents new requests while a trip is currently active.

2. Controller Application (Controlador)

    Concurrency Management:

        Uses Threads to manage the simulated clock, admin command input, and incoming client connections.

        Protects shared data with Mutexes (e.g., scheduling list, client list).

    Process Management:

        Uses fork() and exec() to dynamically launch Veiculo processes.

        Implements I/O redirection (dup2) to capture vehicle telemetry via anonymous pipes.

        Signal handling (kill, SIGUSR1) for forced cancellation of services.

    Environment Variables: Fleet size configuration via NVEICULOS.

3. Vehicle Application (Veiculo)

    Trip Simulation: Simulates movement and travel time.

    Bidirectional Communication:

        Sends telemetry (INICIO, ANDAMENTO, CONCLUIDO) to the Controller via stdout (piped).

        Notifies the Client directly via Named Pipes.

    Signal Handling: Reacts to SIGUSR1 signals to abort trips immediately.

Technologies & Mechanisms

    Language: C

    IPC: Named Pipes (FIFOs), Anonymous Pipes, Signals.

    Synchronization: POSIX Threads (pthread), Mutexes.

    System: Linux / UNIX.

How to Compile and Run
1. Compilation

Use the provided Makefile to compile all modules:
Bash

make

2. Run the Controller

You must define the fleet size environment variable before starting.
Bash

export NVEICULOS=5
./Controlador

3. Run Clients

In a separate terminal (you can open multiple terminals to simulate various clients):
Bash

./Cliente <username>
# Example:
./Cliente ana

Available Commands

Client Commands:

    agendar <time> <location> <distance> (e.g., agendar 10 ISEC 5.5) - Schedule a new trip.

    consultar <id> - Check the status of a specific request.

    cancelar <id> - Cancel a request (Use 0 to cancel all your pending requests).

    sair - Exit the taxi (if in a trip) or close the application.

    terminar - Terminate the session completely.

Controller Commands (Admin):

    listar - List all scheduled services.

    frota - Show the status of vehicles currently running.

    utiliz - List connected users.

    cancelar <id> - Administratively cancel a service.

    terminar - Shut down the platform and notify all clients.

Authors

    Guilherme Costa (2022144234)

    Ricardo Miguel (2022135245)

Instituto Superior de Engenharia de Coimbra - Computer Engineering
