class Value_modification {
public:
  virtual ~Value_modification() = default;
  virtual auto calculate(int v) -> int = 0;
};
