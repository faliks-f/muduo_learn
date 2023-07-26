#ifndef MUDUO_LEARN_NONECOPYABLE_H
#define MUDUO_LEARN_NONECOPYABLE_H

namespace faliks {
    class NoneCopyable {
    public:
        NoneCopyable(const NoneCopyable &) = delete;

        void operator=(const NoneCopyable &) = delete;

    protected:
        NoneCopyable() = default;

        ~NoneCopyable() = default;
    };
}

#endif //MUDUO_LEARN_NONECOPYABLE_H
