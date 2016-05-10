#!/usr/bin/python

import cplex
from cplex.exceptions import CplexError

class Seed:

    def __init__(self, seed):
        self.Q = []
        for j, p in enumerate(list(seed)):
            if p == '1':
                self.Q.append(j)

    def __iter__(self):
        for j in self.Q:
            yield j
    
    def weight(self):
        return len(self.Q)
    
    def span(self):
        return self.Q[-1] + 1

    def occurrences(self, m):
        return m - self.span() + 1

    def __minerror_initialize__(self, instance, m):
        w = self.weight()
        c = self.occurrences(m)
        
        #instance.parameters.mip.cuts.covers = 3
        instance.objective.set_sense(instance.objective.sense.minimize)

        # x_i binary, i = [0 .. m-1]
        instance.variables.add(obj = [1]*m, types = [instance.variables.type.binary]*m)
        
        occurrences = []
        for k in range(c):
            indices = []
            for j in self:
                indices.append(k+j)
            occurrences.append(cplex.SparsePair(ind = indices, val = [1]*w))
        instance.linear_constraints.add(lin_expr = occurrences, senses = ['G']*c, rhs = [1]*c)

    def minerror(self, m):
        try:
            instance = cplex.Cplex()
            self.__minerror_initialize__(instance, m)        
            instance.solve()
            #instance.write("minerror.lp")
        except CplexError, e:
            print e
            return undef
        #print "Similarity (Negated) = ", instance.solution.get_values()
        return instance.solution.get_objective_value()

    def __threshold_initialize__(self, instance, m, k):
        instance.objective.set_sense(instance.objective.sense.maximize)
        
        # x_i binary, i = [0 .. m-1]
        x_i = []
        for i in range(m):
            x_i.append('x_' + str(i))
        instance.variables.add(types = [instance.variables.type.binary]*m, names = x_i)
        
        # sum{x_i} <= k
        instance.linear_constraints.add(lin_expr = [ cplex.SparsePair(ind = x_i, val = [1]*m) ], senses = ['L'], rhs = [k])
        
        w = self.weight()
        s = self.span()
        c = self.occurrences(m)
        
        # c_k binary
        c_k = []
        for k in range(c):
            c_k.append('c_' + str(k))
        instance.variables.add(obj = [1]*c, types = [instance.variables.type.binary]*c, names = c_k)
        
        for k in range(c):
            # sum{x_l}{s.t. x_l in c_k} >= c_k
            x_l = []
            for j in Q:
                x_l.append('x_' + str(k+j))
            instance.linear_constraints.add(lin_expr = [ cplex.SparsePair(ind = x_l+[c_k[k]], val = [1]*w +[-1]) ], senses = ['G'], rhs = [0])
    
    def threshold(self, m, k):
        try:
            instance = cplex.Cplex()
            instance.setOut(env.getNullStream())
            self.__threshold_initialize__(instance, m, k)        
            instance.solve()
            #instance.write("threshold.lp")
        except CplexError, e:
            print e
            return undef
        #print "Complementary Threshold = ", instance.solution.get_objective_value()
        #print "Negated Similarity = ", instance.solution.get_values()
        return self.occurrences(m) - instance.solution.get_objective_value()


class SeedsFamily:

    def __init__(self, seeds):
        self.F = []
        for seed in seeds:
            self.__add__(Seed(seed))

    def __add__(self, Q):
        self.F.append(Q)

    def __iter__(self):
        for Q in self.F:
            yield Q

    def weight(self):
        return undef

    def span(self):
        return undef

    def occurrences(self, m):
        occurrences = 0
        for Q in self.F:
            occurrences += Q.occurrences(m)
        return occurrences

    def __minerror_initialize__(self, instance, m):        
        instance.objective.set_sense(instance.objective.sense.minimize)
        
        # x_i binary, i = [0 .. m-1]
        instance.variables.add(obj = [1]*m, types = [instance.variables.type.binary]*m)
        
        for f, Q in enumerate(self):
            w = Q.weight()
            c = Q.occurrences(m)
            occurrences = []
            for k in range(c):
                indices = []
                for j in Q:
                    indices.append(k+j)
                occurrences.append(cplex.SparsePair(ind = indices, val = [1]*w))
            instance.linear_constraints.add(lin_expr = occurrences, senses = ['G']*c, rhs = [1]*c)
    
    def minerror(self, m):
        try:
            instance = cplex.Cplex()
            self.__minerror_initialize__(instance, m)        
            instance.solve()
        except CplexError, e:
            print e
            return undef
        return instance.solution.get_objective_value()
        
    def threshold(self, m, k):
        try:
            instance = cplex.Cplex()
            self.__threshold_initialize__(instance, m, k)        
            instance.solve()
        except CplexError, e:
            print e
            return undef
        return self.occurrences(m) - instance.solution.get_objective_value()

    def __threshold_initialize__(self, instance, m, k):
        instance.objective.set_sense(instance.objective.sense.maximize)

        # x_i binary, i = [0 .. m-1]
        x_i = []
        for i in range(m):
            x_i.append('x_' + str(i))
        instance.variables.add(types = [instance.variables.type.binary]*m, names = x_i)

        # sum{x_i} <= k
        instance.linear_constraints.add(lin_expr = [ cplex.SparsePair(ind = x_i, val = [1]*m) ], senses = ['L'], rhs = [k])

        for f, Q in enumerate(self):
            w = Q.weight()
            s = Q.span()
            c = Q.occurrences(m)
        
            # c_fk binary
            c_fk = []
            for k in range(c):
                c_fk.append('c_' + str(f) + str(k))
            instance.variables.add(obj = [1]*c, types = [instance.variables.type.binary]*c, names = c_fk)

            for k in range(c):
                # sum{x_l}{s.t. x_l in c_fk} >= c_fk
                x_l = []
                for j in Q:
                    x_l.append('x_' + str(k+j))
                instance.linear_constraints.add(lin_expr = [ cplex.SparsePair(ind = x_l+[c_fk[k]], val = [1]*w +[-1]) ], senses = ['G'], rhs = [0])

