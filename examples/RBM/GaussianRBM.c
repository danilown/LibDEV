#include "dev.h"

int main(int argc, char **argv)
{
    if (argc != 11)
    {
        fprintf(stderr, "\nUsage: GaussianRBM <training set> <testing set> <output results file name> <cross-validation iteration number> \
                <search space configuration file> <output best parameters file name> <n_epochs> <batch_size> \
                <number of iterations for Constrastive Divergence> <variance value>");
        exit(-1);
    }

    SearchSpace *s = NULL;
    int i, j, z;
    int iteration = atoi(argv[4]), n_epochs = atoi(argv[7]), batch_size = atoi(argv[8]), n_gibbs_sampling = atoi(argv[9]);
    int n_hidden_units;
    double variance = atof(argv[10]);
    double *eta_bound, errorTrain, errorTest, *sigma;
    FILE *f = NULL;
    Subgraph *Train = NULL, *Test = NULL;
    Dataset *DatasetTrain = NULL, *DatasetTest = NULL;
    RBM *m = NULL;

    Train = ReadSubgraph(argv[1]);
    Test = ReadSubgraph(argv[2]);
    DatasetTrain = Subgraph2Dataset(Train);
    DatasetTest = Subgraph2Dataset(Test);

    s = ReadSearchSpaceFromFile(argv[5], _PSO_);

    eta_bound = (double *)calloc(2, sizeof(double));
    eta_bound[0] = s->LB[1];
    eta_bound[1] = s->UB[1];

    fprintf(stderr, "\nInitializing search space ... ");
    InitializeSearchSpace(s, _PSO_);
    fprintf(stderr, "\nOk\n");

    sigma = (double *)calloc(Train->nfeats, sizeof(double));

    for (i = 0; i < Train->nfeats; i++)
        sigma[i] = variance;

    fprintf(stderr, "\nRunning PSO ... ");
    runPSO(s, Gaussian_BernoulliRBM, Train, n_epochs, batch_size, n_gibbs_sampling, sigma, eta_bound);

    fprintf(stderr, "\n\nRunning Gaussian RBM with best parameters on training set ... ");
    n_hidden_units = (int)s->g[0];
    m = CreateNewDRBM(Train->nfeats, n_hidden_units, Train->nlabels, sigma);
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

    errorTrain = GaussianBernoulliRBMTrainingbyContrastiveDivergence(DatasetTrain, m, n_epochs, n_gibbs_sampling, batch_size);

    fprintf(stderr, "\n\nRunning Gaussian RBM for reconstruction on testing set ... ");
    errorTest = GaussianBernoulliRBMReconstruction(DatasetTest, m);
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

    free(eta_bound);
    free(sigma);
    DestroySearchSpace(&s, _PSO_);
    DestroyDataset(&DatasetTrain);
    DestroyDataset(&DatasetTest);
    DestroySubgraph(&Train);
    DestroySubgraph(&Test);
    DestroyDRBM(&m);

    return 0;
}