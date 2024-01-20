
int foam_main(int argc, char **args);
void step();

void run() {
    for(;;) step();
}

int main(int argc, char **args) {
    return foam_main(argc, args);
}
