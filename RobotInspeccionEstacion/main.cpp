using namespace System;
using namespace System::Collections;
using namespace System::Threading;

// Clase auxiliar para representar posiciones
ref class Punto {
public:
    int x;
    int y;
    Punto(int x, int y) {
        this->x = x;
        this->y = y;
    }
};

ref class FactoryStats {
public:
    // Mantiene el contador de inspecciones realizadas
    static int totalInspecciones = 0;
};

ref class FactoryLayout {
public:
    // Tablero de celdas donde 0 es vac�o, otros valores indican estaciones
    static array<int, 2>^ board = {
        {0, 1, 0, 2, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 3, 0, 4, 0, 0},
        {0, 5, 0, 6, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 7, 0, 8, 0, 0},
        {0, 9, 0, 10, 0, 0, 0, 0, 0, 0}
    };

    // Muestra el tablero en consola con la posici�n actual del robot
    static void MostrarTablero(int rx, int ry) {
        for (int i = 0; i < board->GetLength(0); i++) {
            for (int j = 0; j < board->GetLength(1); j++) {
                if (i == rx && j == ry)
                    Console::Write("R "); // "R" representa la posici�n del robot
                else
                    Console::Write(board[i, j] + " "); // Mostrar las estaciones y vac�os
            }
            Console::WriteLine();
        }
    }

    // M�todo para encontrar las coordenadas de la estaci�n en el tablero
    static Punto^ ObtenerCoordenadasDeEstacion(int numeroEstacion) {
        for (int i = 0; i < board->GetLength(0); i++) {
            for (int j = 0; j < board->GetLength(1); j++) {
                if (board[i, j] == numeroEstacion) {
                    return gcnew Punto(i, j); // Retorna las coordenadas (x, y)
                }
            }
        }
        return nullptr; // Si no se encuentra la estaci�n, retorna nullptr
    }
};

ref class Robot {
protected:
    int x, y;
    int inspecciones;
    Hashtable^ estadoEstaciones;

public:
    // Constructor que inicializa la posici�n del robot y las estaciones
    Robot() {
        x = 0;
        y = 0;
        inspecciones = 0;
        estadoEstaciones = gcnew Hashtable();
        for (int i = 1; i <= 10; i++) {
            estadoEstaciones[i] = "funcional";
        }
    }

    // Mueve el robot paso a paso hasta una posici�n de destino
    bool MoverPasoAPaso(int tx, int ty) {
        Queue^ cola = gcnew Queue();
        Hashtable^ visitado = gcnew Hashtable();
        Hashtable^ previo = gcnew Hashtable();

        cola->Enqueue(gcnew Punto(x, y));
        visitado[String::Format("{0},{1}", x, y)] = true;

        // Movimientos posibles (arriba, abajo, izquierda, derecha)
        array<int>^ dx = { -1, 1, 0, 0 };
        array<int>^ dy = { 0, 0, -1, 1 };

        bool encontrado = false;

        while (cola->Count > 0) {
            Punto^ actual = (Punto^)cola->Dequeue();

            // Si llegamos al destino, paramos
            if (actual->x == tx && actual->y == ty) {
                encontrado = true;
                break;
            }

            // Intentamos movernos a las celdas adyacentes
            for (int i = 0; i < 4; i++) {
                int nx = actual->x + dx[i];
                int ny = actual->y + dy[i];

                // Verificar que la celda est� dentro del tablero y sea vac�a
                if (nx >= 0 && nx < 5 && ny >= 0 && ny < 10 &&
                    FactoryLayout::board[nx, ny] == 0 &&
                    !visitado->ContainsKey(String::Format("{0},{1}", nx, ny))) {

                    Punto^ siguiente = gcnew Punto(nx, ny);
                    cola->Enqueue(siguiente);
                    visitado[String::Format("{0},{1}", nx, ny)] = true;
                    previo[String::Format("{0},{1}", nx, ny)] = actual;
                }
            }
        }

        if (!encontrado) return false;

        // reconstruir el camino
        System::Collections::Generic::List<Punto^>^ camino = gcnew System::Collections::Generic::List<Punto^>();
        Punto^ paso = gcnew Punto(tx, ty);
        String^ clave = String::Format("{0},{1}", tx, ty);

        while (!(paso->x == x && paso->y == y)) {
            camino->Insert(0, paso);
            paso = (Punto^)previo[clave];
            clave = String::Format("{0},{1}", paso->x, paso->y);
        }

        // Mostrar cada paso del robot
        for each (Punto ^ p in camino) {
            x = p->x;
            y = p->y;
            FactoryLayout::MostrarTablero(x, y);
            Console::WriteLine("Movimiento -> ({0}, {1})", x, y);
            Console::WriteLine();
            Thread::Sleep(500); // Espera para ver el movimiento
        }
        return true;
    }

    // Inspeccionar la estaci�n en la posici�n dada
    void InspeccionarEstacion(int numeroEstacion) {
        // Obtener las coordenadas de la estaci�n a inspeccionar
        Punto^ coordenadas = FactoryLayout::ObtenerCoordenadasDeEstacion(numeroEstacion);
        if (coordenadas == nullptr) {
            Console::WriteLine("Estaci�n {0} no encontrada en el tablero.", numeroEstacion);
            return;
        }

        int ex = coordenadas->x;
        int ey = coordenadas->y;

        array<int>^ dx = { -1, 1, 0, 0 };
        array<int>^ dy = { 0, 0, -1, 1 };

        // Buscar una celda vac�a adyacente a la estaci�n
        for (int i = 0; i < 4; i++) {
            int ax = ex + dx[i];
            int ay = ey + dy[i];

            if (ax >= 0 && ax < 5 && ay >= 0 && ay < 10 &&
                FactoryLayout::board[ax, ay] == 0) {

                // Mover el robot al destino
                bool llego = MoverPasoAPaso(ax, ay);

                // Si el robot lleg�, se realiza la inspecci�n
                if (llego) {
                    Console::ForegroundColor = ConsoleColor::Green;
                    Console::WriteLine("InspectorBot inspeccionando estaci�n {0}. Estado: {1}",
                        numeroEstacion, estadoEstaciones[numeroEstacion]);
                    Console::ResetColor();
                    inspecciones++;
                    FactoryStats::totalInspecciones++;
                }
                else {
                    Console::WriteLine("No se pudo alcanzar la estaci�n {0}", numeroEstacion);
                }
                return;
            }
        }
        Console::WriteLine("No hay celdas vac�as adyacentes a la estaci�n {0}", numeroEstacion);
    }

    // Comando de detenci�n
    void Detener() {
        Console::WriteLine("El robot se ha detenido.");
    }

    // Comando de recarga
    void Recargar() {
        Console::WriteLine("El robot se ha recargado.");
    }

    // Comando de reporte de inspecci�n
    void Reportar() {
        Console::WriteLine("Reporte de inspecci�n: Total de inspecciones realizadas: {0}", FactoryStats::totalInspecciones);
    }
};

// M�todo principal que maneja la interacci�n con el usuario
int main(array<System::String^>^ args) {
    // Crear el robot
    Robot^ robot = gcnew Robot();
    bool bInicio = true;

    // Comando de entrada
    String^ command = "";
    Console::WriteLine("Bienvenido a InspectorBot!");
    Console::WriteLine("Comandos disponibles: inspeccionar, detener, recargar, reportar, Q para salir");

    while (command != "Q") {
        // Solicitar comando al usuario
        Console::Write("Ingrese comando (inspeccionar, detener, recargar, reportar, Q para salir): ");
        command = Console::ReadLine();

        // Procesar comandos
        if (command == "inspeccionar") {
            int numeroEstacion = 0;
            bool estacionValida = false;

            // Solicitar el n�mero de estaci�n (1-10)
            while (!estacionValida) {
                Console::Write("Ingrese el n�mero de estaci�n (1-10): ");
                try {
                    numeroEstacion = Convert::ToInt32(Console::ReadLine());
                    if (numeroEstacion >= 1 && numeroEstacion <= 10) {
                        estacionValida = true;
                    }
                    else {
                        Console::WriteLine("N�mero de estaci�n no v�lido. Intente nuevamente.");
                    }
                }
                catch (FormatException^) {
                    Console::WriteLine("Entrada no v�lida. Debe ingresar un n�mero.");
                }
            }
            if (bInicio)
            {
                /*Se muestra el tablero al Inicio*/
                Console::WriteLine("Inspeccionado estaci�n:");
                Console::WriteLine("======================");
                FactoryLayout::MostrarTablero(0, 0);
                Console::WriteLine("Inicio: (0, 0)");
                Console::WriteLine();
                bInicio = false;
                Thread::Sleep(500); // Espera para ver el movimiento
            }
            // Llamar al m�todo de inspecci�n
            robot->InspeccionarEstacion(numeroEstacion);
        }
        else if (command == "detener") {
            robot->Detener();
        }
        else if (command == "recargar") {
            robot->Recargar();
        }
        else if (command == "reportar") {
            robot->Reportar();
        }
        else if (command == "Q") {
            break; // Salir del ciclo
        }
        else {
            Console::WriteLine("Comando no reconocido. Intente nuevamente.");
        }
    }

    // Mostrar mensaje final con el total de inspecciones
    Console::WriteLine("Saliendo... Inspecciones totales realizadas: {0}", FactoryStats::totalInspecciones);
    return 0;
}
