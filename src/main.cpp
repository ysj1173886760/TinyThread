#include "master.h"
#include "logger.h"
#include <iostream>
#include <algorithm>
#include <random>

struct Arg {
    int left, right;
    int *array;
};

void quick_sort(void *args) {
    Arg *arg = (Arg*)args;
    LOG_INFO("%d %d", arg->left, arg->right);
    if (arg->right - arg->left < 10) {
        std::sort(arg->array + arg->left, arg->array + arg->right);
    } else {
        int middle = (arg->right + arg->left) / 2;
        Arg *left_arg = new Arg;
        Arg *right_arg = new Arg;
        left_arg->left = arg->left;
        left_arg->right = middle;
        right_arg->left = middle + 1;
        right_arg->right = arg->right;
        left_arg->array = arg->array;
        right_arg->array = arg->array;

        create(quick_sort, left_arg);
        quick_sort(right_arg);
    }
}

int main() {
    initialize();
    int max_size = 100;
    int *a = new int[max_size];
    for (int i = 0; i < max_size; i++) {
        a[i] = rand() % 100;
    }
    Arg *arg = new Arg;
    arg->left = 0;
    arg->right = max_size;
    arg->array = a;
    create(quick_sort, arg);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    for (int i = 0; i < max_size; i++) {
        std::cout << a[i] << " ";
    }
    return 0;
}