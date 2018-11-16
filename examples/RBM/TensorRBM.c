#include "dev.h"

int main(int argc, char **argv)
{
    if (argc != 11)
    {
        fprintf(stderr, "\nUsage: TensorRBM <training set> <testing set> <output results file name> <cross-validation iteration number> \
                <search space configuration file> <output best parameters file name> <n_epochs> <batch_size> \
                <number of iterations for Constrastive Divergence> <1 - CD | 2 - PCD | 3 - FPCD>");
        exit(-1);
    }

    SearchSpace *s = NULL;
    int i, j, z;
    int iteration = atoi(argv[4]), n_epochs = atoi(argv[7]), batch_size = atoi(argv[8]), n_gibbs_sampling = atoi(argv[9]), op = atoi(argv[10]);
    int n_hidden_units;
    double *eta_bound, errorTrain, errorTest;
    FILE *f = NULL;
    Subgraph *Train = NULL, *Test = NULL;
    Dataset *DatasetTrain = NULL, *DatasetTest = NULL;
    RBM *m = NULL;

    Train = ReadSubgraph(argv[1]);
    Test = ReadSubgraph(argv[2]);
    DatasetTrain = Subgraph2Dataset(Train);
    DatasetTest = Subgraph2Dataset(Test);

    s = ReadSearchSpaceFromFile(argv[5], _HS_);

    eta_bound = (double *)calloc(2, sizeof(double));
    eta_bound[0] = s->LB[1];
    eta_bound[1] = s->UB[1];

    s->t_g = CreateTensor(s->n, _QUATERNION_);
    for (i = 0; i < s->m; i++)
    {
        s->a[i]->t = CreateTensor(s->n, _QUATERNION_); /* It allocates a new tensor for each agent */
    }

    fprintf(stderr, "\nInitializing search space ... ");
    InitializeTensorSearchSpace(s, _QUATERNION_);
    fprintf(stderr, "\nOk\n");

    fprintf(stderr, "\nRunning TensorHS ... ");
    runTensorHS(s, _QUATERNION_, BernoulliRBM, Train, op, n_epochs, batch_size, n_gibbs_sampling, eta_bound);

    fprintf(stderr, "\n\nRunning TensorRBM with best parameters on training set ... ");
    n_hidden_units = (int)s->g[0];
    m = CreateRBM(Train->nfeats, n_hidden_units, Train->nlabels);
    m->eta = s->g[1];
    m->lambda = s->g[2];
    m->alpha = s->g[3];
    m->eta_min = eta_bound[0];
    m->eta_max = eta_bound[1];

    InitializeWeights(m);
    InitializeLabelWeights(m);
    InitializeBias4HiddenUnits(m);
    InitializeBias4VisibleUnitsWithRandomValues(m);
    InitializeBias4LabelUnits(m);

    switch (op)
    {
    case 1:
        errorTrain = BernoulliRBMTrainingbyContrastiveDivergence(DatasetTrain, m, n_epochs, 1, batch_size);
        break;
    case 2:
        errorTrain = BernoulliRBMTrainingbyPersistentContrastiveDivergence(DatasetTrain, m, n_epochs, n_gibbs_sampling, batch_size);
        break;
    case 3:
        errorTrain = BernoulliRBMTrainingbyFastPersistentContrastiveDivergence(DatasetTrain, m, n_epochs, n_gibbs_sampling, batch_size);
        break;
    }

    fprintf(stderr, "\n\nRunning TensorRBM for reconstruction on testing set ... ");
    errorTest = BernoulliRBMReconstruction(DatasetTest, m);
    fprintf(stderr, "\nOK\n");

    fprintf(stderr, "\nTraining error: %lf\nTesting error: %lf\n", errorTrain, errorTest);

    fprintf(stderr, "\nSaving outputs ... ");
    f = fopen(argv[3], "a");
    fprintf(f, "%d %lf %lf\n", iteration, errorTrain, errorTest);
    fclose(f);

    f = fopen(argv[6], "a");
    fprintf(f, "%d ", s->n);
    for (i = 0; i < s->n; i++)
        fprintf(f, "%lf ", s->g[i]);
    fprintf(f, "\n");
    fclose(f);
    fprintf(stderr, "Ok!\n");

    DestroyTensor(&s->t_g, s->n);
    for (i = 0; i < s->m; i++)
    {
        DestroyTensor(&s->a[i]->t, s->n); /* It deallocates the tensor for each agent */
    }

    free(eta_bound);
    DestroySearchSpace(&s, _HS_);
    DestroyDataset(&DatasetTrain);
    DestroyDataset(&DatasetTest);
    DestroySubgraph(&Train);
    DestroySubgraph(&Test);
    DestroyRBM(&m);

    return 0;
}
