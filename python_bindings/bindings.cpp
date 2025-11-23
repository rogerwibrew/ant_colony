#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "City.h"
#include "Graph.h"
#include "Tour.h"
#include "TSPLoader.h"
#include "PheromoneMatrix.h"
#include "Ant.h"
#include "AntColony.h"

namespace py = pybind11;

PYBIND11_MODULE(aco_solver, m) {
    m.doc() = "Ant Colony Optimization TSP Solver Python Bindings";

    // City class
    py::class_<City>(m, "City")
        .def(py::init<int, double, double>(),
             py::arg("id"),
             py::arg("x"),
             py::arg("y"),
             "Construct a city with id and coordinates")
        .def("distanceTo", &City::distanceTo,
             py::arg("other"),
             "Calculate Euclidean distance to another city")
        .def("getId", &City::getId,
             "Get city ID")
        .def("getX", &City::getX,
             "Get X coordinate")
        .def("getY", &City::getY,
             "Get Y coordinate")
        .def("__repr__", [](const City &c) {
            return "<City id=" + std::to_string(c.getId()) +
                   " x=" + std::to_string(c.getX()) +
                   " y=" + std::to_string(c.getY()) + ">";
        });

    // Graph class
    py::class_<Graph>(m, "Graph")
        .def(py::init<const std::vector<City>&>(),
             py::arg("cities"),
             "Construct graph from list of cities")
        .def(py::init<>(),
             "Construct empty graph")
        .def("getDistance", &Graph::getDistance,
             py::arg("cityA"),
             py::arg("cityB"),
             "Get distance between two cities (O(1) lookup)")
        .def("getNumCities", &Graph::getNumCities,
             "Get total number of cities")
        .def("getCity", &Graph::getCity,
             py::arg("index"),
             "Get city by index")
        .def("getCities", &Graph::getCities,
             "Get all cities as a list")
        .def("isValid", &Graph::isValid,
             "Check if graph has cities")
        .def("nearestNeighborTourLength", &Graph::nearestNeighborTourLength,
             py::arg("startCity") = 0,
             "Calculate tour length using greedy nearest neighbor heuristic")
        .def("__repr__", [](const Graph &g) {
            return "<Graph cities=" + std::to_string(g.getNumCities()) + ">";
        });

    // Tour class
    py::class_<Tour>(m, "Tour")
        .def(py::init<>(),
             "Construct empty tour")
        .def(py::init<const std::vector<int>&, double>(),
             py::arg("sequence"),
             py::arg("distance"),
             "Construct tour with sequence and distance")
        .def("setTour", &Tour::setTour,
             py::arg("sequence"),
             py::arg("distance"),
             "Set tour sequence and distance")
        .def("getDistance", &Tour::getDistance,
             "Get total tour distance")
        .def("getSequence", &Tour::getSequence,
             "Get city visit sequence")
        .def("validate", &Tour::validate,
             py::arg("numCities"),
             "Verify tour visits all cities exactly once")
        .def("__repr__", [](const Tour &t) {
            return "<Tour distance=" + std::to_string(t.getDistance()) +
                   " cities=" + std::to_string(t.getSequence().size()) + ">";
        });

    // TSPLoader class
    py::class_<TSPLoader>(m, "TSPLoader")
        .def(py::init<const std::string&>(),
             py::arg("filename"),
             "Construct loader for TSP file (auto-searches data/ directories)")
        .def("loadGraph", &TSPLoader::loadGraph,
             "Load graph from file (auto-detects format)")
        .def_static("loadFromCoordinates", &TSPLoader::loadFromCoordinates,
             py::arg("filename"),
             "Load graph from coordinate file")
        .def_static("loadFromDistanceMatrix", &TSPLoader::loadFromDistanceMatrix,
             py::arg("filename"),
             "Load graph from distance matrix file")
        .def_static("loadFromTSPLIB", &TSPLoader::loadFromTSPLIB,
             py::arg("filename"),
             "Load graph from TSPLIB format file");

    // PheromoneMatrix class
    py::class_<PheromoneMatrix>(m, "PheromoneMatrix")
        .def(py::init<int, double>(),
             py::arg("numCities"),
             py::arg("initialPheromone"),
             "Construct pheromone matrix")
        .def("initialize", &PheromoneMatrix::initialize,
             py::arg("value"),
             "Reset all pheromones to specified value")
        .def("getPheromone", &PheromoneMatrix::getPheromone,
             py::arg("cityA"),
             py::arg("cityB"),
             "Get pheromone level between two cities")
        .def("setPheromone", &PheromoneMatrix::setPheromone,
             py::arg("cityA"),
             py::arg("cityB"),
             py::arg("value"),
             "Set pheromone level between two cities")
        .def("evaporate", &PheromoneMatrix::evaporate,
             py::arg("rho"),
             "Apply evaporation to all edges")
        .def("depositPheromone", &PheromoneMatrix::depositPheromone,
             py::arg("cityA"),
             py::arg("cityB"),
             py::arg("amount"),
             "Add pheromone to edge")
        .def("clampPheromones", &PheromoneMatrix::clampPheromones,
             "Enforce min/max bounds (for MMAS variant)")
        .def("getMinPheromone", &PheromoneMatrix::getMinPheromone,
             "Get minimum pheromone bound")
        .def("getMaxPheromone", &PheromoneMatrix::getMaxPheromone,
             "Get maximum pheromone bound");

    // Ant class
    py::class_<Ant>(m, "Ant")
        .def(py::init<int, int>(),
             py::arg("startCity"),
             py::arg("numCities"),
             "Construct ant starting at specified city")
        .def("reset", &Ant::reset,
             py::arg("startCity"),
             "Reset ant to start new tour from specified city")
        .def("selectNextCity", &Ant::selectNextCity,
             py::arg("graph"),
             py::arg("pheromones"),
             py::arg("alpha"),
             py::arg("beta"),
             "Choose next city probabilistically")
        .def("visitCity", &Ant::visitCity,
             py::arg("city"),
             py::arg("graph"),
             "Add city to tour")
        .def("hasVisitedAll", &Ant::hasVisitedAll,
             "Check if tour is complete")
        .def("completeTour", &Ant::completeTour,
             py::arg("graph"),
             "Finalize and return the tour")
        .def("calculateTourLength", &Ant::calculateTourLength,
             py::arg("graph"),
             "Calculate total tour distance");

    // AntColony class with callback support
    py::class_<AntColony>(m, "AntColony")
        .def(py::init<const Graph&, int, double, double, double, double, bool>(),
             py::arg("graph"),
             py::arg("numAnts") = 20,
             py::arg("alpha") = 1.0,
             py::arg("beta") = 2.0,
             py::arg("rho") = 0.5,
             py::arg("Q") = 100.0,
             py::arg("useDistinctStartCities") = false,
             "Construct ant colony optimizer\n\n"
             "Parameters:\n"
             "  graph: TSP problem instance\n"
             "  numAnts: Number of ants in colony (default: 20)\n"
             "  alpha: Pheromone importance factor (default: 1.0)\n"
             "  beta: Heuristic importance factor (default: 2.0)\n"
             "  rho: Evaporation rate (default: 0.5)\n"
             "  Q: Pheromone deposit factor (default: 100.0)\n"
             "  useDistinctStartCities: Each ant starts at different city (default: False)")
        .def("initialize", &AntColony::initialize,
             "Initialize pheromones and ants")
        .def("runIteration", &AntColony::runIteration,
             "Execute one complete iteration")
        .def("constructSolutions", &AntColony::constructSolutions,
             "All ants build tours")
        .def("updatePheromones", &AntColony::updatePheromones,
             "Evaporate and deposit pheromones")
        .def("solve", &AntColony::solve,
             py::arg("maxIterations"),
             py::arg("callback") = nullptr,
             py::call_guard<py::gil_scoped_release>(),  // Release GIL during C++ computation
             "Run algorithm for specified iterations\n\n"
             "Parameters:\n"
             "  maxIterations: Number of iterations to run (or -1 for auto-convergence)\n"
             "  callback: Optional progress callback function\n\n"
             "Returns:\n"
             "  Best tour found")
        .def("getBestTour", &AntColony::getBestTour,
             "Get best solution found")
        .def("getConvergenceData", &AntColony::getConvergenceData,
             "Get iteration history")
        .def("setProgressCallback", &AntColony::setProgressCallback,
             py::arg("callback"),
             "Set progress callback function")
        .def("setCallbackInterval", &AntColony::setCallbackInterval,
             py::arg("interval"),
             "Set callback interval (default: 10 iterations)")
        .def("setConvergenceThreshold", &AntColony::setConvergenceThreshold,
             py::arg("threshold"),
             "Set convergence threshold (default: 200 iterations without improvement)")
        .def("setUseParallel", &AntColony::setUseParallel,
             py::arg("useParallel"),
             "Enable/disable parallel execution (default: True if OpenMP available)\n\n"
             "Parameters:\n"
             "  useParallel: True to enable multi-threading, False for serial execution")
        .def("setNumThreads", &AntColony::setNumThreads,
             py::arg("numThreads"),
             "Set number of threads for parallel execution\n\n"
             "Parameters:\n"
             "  numThreads: 0=auto-detect, 1=serial, 2+=specific thread count\n\n"
             "Note: Only effective if OpenMP is available and useParallel is True")
        .def("getNumAnts", &AntColony::getNumAnts,
             "Get number of ants")
        .def("getAlpha", &AntColony::getAlpha,
             "Get alpha parameter")
        .def("getBeta", &AntColony::getBeta,
             "Get beta parameter")
        .def("getRho", &AntColony::getRho,
             "Get rho parameter")
        .def("getQ", &AntColony::getQ,
             "Get Q parameter")
        .def("__repr__", [](const AntColony &ac) {
            return "<AntColony ants=" + std::to_string(ac.getNumAnts()) +
                   " alpha=" + std::to_string(ac.getAlpha()) +
                   " beta=" + std::to_string(ac.getBeta()) + ">";
        });
}
