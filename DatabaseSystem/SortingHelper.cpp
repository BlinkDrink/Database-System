#include "SortingHelper.h"

void heapify(vector<Record>& arr, int n, int i, int columnId)
{
	int largest = i;
	int l = 2 * i + 1;
	int r = 2 * i + 2;

	if (l < n && arr[l].get(columnId) > arr[largest].get(columnId))
		largest = l;

	if (r < n && arr[r].get(columnId) > arr[largest].get(columnId))
		largest = r;

	if (largest != i) {
		std::swap(arr[i], arr[largest]);

		heapify(arr, n, largest, columnId);
	}
}

void heapSort(vector<Record>& arr, int columnId)
{
	int n = arr.size();
	for (int i = n / 2 - 1; i >= 0; i--)
		heapify(arr, n, i, columnId);

	for (int i = n - 1; i > 0; i--) {
		std::swap(arr[0], arr[i]);
		heapify(arr, i, 0, columnId);
	}
}