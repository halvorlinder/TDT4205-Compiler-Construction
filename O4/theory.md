func main()
    iter = 2
start:
    t1 = iter < 40
    ifFalse t1 goto end
        param iter 
        count = call collatz, 1
        param iter
        param "converges in"
        param count
        param "steps"
        iter = iter + 1
    goto start 
end:
    return 0

func collatz(n)
    steps = 0
start:
    t1 = n > 1
    ifFalse t1 goto end1
        i = n/2
        t2 = i*2
        ifFalse t2 = n goto else
            n = i
        goto end2
else:
            t3 = 3*n
            t4 = t3 + 1
            n = t4
end2:
        steps = steps + 1
        goto start 
    return steps
end:
