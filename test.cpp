int main() {
  int i = 7;
  int j = 13;
  while (i % j != 1) {
    i++;
    j--;
  }
  return j;
}