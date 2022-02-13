#pragma once
#include "Record.hpp"

void heapSort(vector<Record>& arr, int columnId);

void heapify(vector<Record>& arr, int n, int i, int columnId);
