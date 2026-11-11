# L'eXPLORE "First Step" project description (initial, 2026)

> Archived 2026-11-06. The initial project description, kept verbatim as the user supplied it. It is context only and is out of date: the sampling design changed a lot on site (depths, time points, sample count). What happened: [field-lexplore-geneva-2026-11-03.md](../../logs/field-lexplore-geneva-2026-11-03.md) §32 and [field-report-draft-2026-11.md](../../field/field-report-draft-2026-11.md).

First Step project
Project title
Daily dynamics of lake bacterial communities
Summary
Photosynthetic microbial communities in the Leman lake may vary depending on the time of the day, as observed for marine plankton. Such variation remains outside the resolution of current methods which may lead to biases. This project aims to monitor the daily (or diel) microbial communities at various depths to account for this effect.

Bacterioplankton drives most of the biogeochemical turnover in lakes, and its composition is structured by depth, light and season. In the Leman Lake, a monthly survey at twelve depths from the LéXPLORE platform showed a strongly layered and seasonally variable community: Planktothrix rubescens dominated the upper 30 m in autumn and winter, the picocyanobacterial genus Cyanobium prevailed near the surface in summer, and non-photosynthetic genera such as CL500-29, Flavobacterium, Rhodoferax and Methylobacter characterised the deeper layers (Carratalà et al. 2023).
However, changes occurring within the hours of a single day are important to understand because the photosynthesis follows irradiance over hours. Consequently, dissolved organic carbon released by primary producers fuels heterotrophic bacteria with a lag, and vertical position within the water column is actively regulated by buoyancy in filamentous cyanobacteria. Whether these processes translate into a measurable diel signature in community composition is an open question: diel periodicity is well documented in marine plankton, while a recent study of a clear boreal lake detected no diel cycling in bacterial community structure at all (Papadopoulou et al. 2025).
At LéXPLORE, physical and optical variables are already recorded continuously, including irradiance, thermistor-chain temperature profiles and high-frequency chlorophyll a fluorescence profiles (Wüest et al. 2021; Minaudo et al. 2021). The biological sampling that accompanies them remains manual and is therefore constrained at a low frequency, missing these diel variations.
The CoWaS (Continuous Water Sampler), the sampling module of the Innosuisse-funded AquaDecode project, removes that constraint. It filters and preserves water in situ on a programmable schedule and can draw from several depths, which makes a diel sampling series at a fixed station operationally realistic for the first time at this site.
In Lake Geneva, internal waves displace stratified layers by several metres within a few hours (Fernández Castro et al. 2021). A change measured at a fixed depth can therefore reflect a layer moving up or down instead of a change in the community. Every sample is time-stamped and matched to the thermal structure recorded continuously by the platform, so it is described by its temperature and its position relative to the thermocline as well as by its nominal depth.
This First Step project applies high-frequency autonomous sampling at LéXPLORE, at several depths, as a bounded test of whether bacterial community composition varies over the diel cycle and whether that variation tracks light.
Aim of the project
The project characterises the diel dynamics of bacterial communities in the water column of the Leman Lake at the LéXPLORE platform, and tests whether they track the light cycle.
Objective 1 : quantifying variation in bacterial community composition at three depths (to be determined more precisely at the beginning of the field collection, but a 2m, 10m and 20m setting could represent a reasonable starting setting) across repeated 48 hour cycles over the course of a month, using 16S rRNA gene metabarcoding on samples collected automatically every 3 to 4 hours.
Objective 2 : test  the association between community composition and the environmental variables recorded continuously by the platform, in particular irradiance, water temperature and chlorophyll a fluorescence, and identify if there is a periodic behaviour among the taxa.
Objective 3 : contrast the magnitude of diel variation with the magnitude of variation among depths, to establish whether sub-daily change is large enough to affect the interpretation of conventional monthly surveys.
The expected outcome is a quantified answer to a design question that applies to every lake eDNA survey: whether the time of day at which a sample is taken changes the community it describes.
Experimental approach

●	Site and access 
The LéXPLORE platform, anchored about 570 m offshore from Pully over 110 m of water, operated by a consortium of five institutions including UNIL. The platform supplies the core environmental dataset used here as covariates.
●	Sampling
One autonomous sampler (CoWaS unit) is deployed with intakes at three depths, provisionally near-surface, within the epilimnion and near the thermocline or deep chlorophyll maximum, with the exact depths set from platform profiles at deployment. Samples are collected every 3 to 4 hours over 48 hour cycles, giving 12 to 16 time points per depth per campaign, with two filter replicates per time point. Two to three campaigns are run within one sampling month.
●	Environmental covariates
Surface light, wind speed and water temperature at fixed depths are recorded continuously at the platform, and the automated profiler adds temperature, light and chlorophyll a fluorescence profiles several times a day. These series come from the LéXPLORE core dataset and are aligned with the timing of each filtration, interpolated between casts where needed.
●	Marker and amplification
Filtration and preservation occur on board CoWaS; extraction takes place at the Naturéum and UNIL. The V3-V4 region of the 16S rRNA gene is targeted with the 341F and 805R primers (Klindworth et al. 2013), matching the existing Leman Lake dataset from previous studies so that the two are directly comparable. Recombinase polymerase amplification (RPA) is the primary route, consistent with the AquaDecode objective of amplification on the station itself: it runs isothermally at around 40 °C and tolerates the inhibitors common in lake extracts. Conventional PCR is kept as a validated fallback and run in parallel on a subset, which also quantifies any divergence in recovered composition between the two chemistries (RPA vs. PCR). Amplicon sequence variants are inferred in DADA2 and assigned against SILVA; absolute and relative bacterial abundances are estimated by 16S qPCR.
●	Tagging and sequencing
Primers carry individual 5' tags so that a whole campaign is pooled into one nanopore flow cell. A combinatorial design of 16 forward by 16 reverse tags addresses up to 256 samples per run, which covers 3-4 full 48 hour cycle at three depths with replicates and controls. Tag jumping creates false tag combinations and inflates apparent diversity (Schnell et al. 2015), so that a subset of combinations is left unused to measure the jump rate directly. Tags are designed at sufficient edit distance for error-tolerant demultiplexing of nanopore reads, and tagged RPA is validated on mock communities beforehand, since 5' extensions affect RPA primers more than PCR primers.
●	Controls
Negatives are central to this design. Blank cartridges are placed at intervals within each CoWaS sequence to measure carry-over between consecutive time points, which would otherwise be read as a diel signal. Filtration, extraction and amplification controls accompany every batch, and detection thresholds for individual taxa are set from the distribution observed in blanks rather than from an arbitrary read cut-off. Positive controls with known sequences will test detection capacities and biases.
●	Analyses
Community dissimilarity is summarised by Bray-Curtis distances, visualised by NMDS and tested by PERMANOVA with time of day, depth and campaign as factors. Variance partitioning separates diel, vertical and between-campaign contributions. Periodicity in individual taxa is tested by harmonic regression or a dedicated rhythmicity test on the time series. Samples are additionally referenced to isotherm depth rather than nominal depth, so that community change can be distinguished from vertical displacement of layers by internal waves.
●	Supervision
Prof. Nadir Alvarez (DEE, UNIL; Naturéum) and Dr Quentin Perrin (UNIL), within the AquaDecode consortium.
Significance
Environmental DNA monitoring is moving from a research tool to routine assessment, and its results are used to compare sites, years and management options. However assessments generally rely on a sampling design inherited from an era of manual fieldwork. For instance in lakes, it is usually restricted to only few measurements per day and at a limited number of depths. However, the assumption behind it, that a community sampled at a given day of a month describes the community during that month, has never been tested extensively in a lake at the resolution needed to check it.
Autonomous sampling removes the constraint of a limited number of sampling sessions. Using a newly designed Continuous Water Sampler (CoWaS), this project aims at measuring how much bacterial community composition changes within a single day and at different depths, at a site where light, temperature and mixing are recorded continuously. If daily variation is small, monthly surveys gain a quantified justification. If it is large, the hour of sampling becomes a variable that becomes an important criterion in sampling protocols, including long-term programmes.


References
 
1.        Carratalà A., Chappelier C., Selmoni O., et al. (2023) Vertical distribution and seasonal dynamics of planktonic cyanobacteria communities in a water column of deep mesotrophic Lake Geneva. Frontiers in Microbiology 14: 1295193. doi:10.3389/fmicb.2023.1295193
2.       Fernández Castro B., Sepúlveda Steiner O., Knapp D., et al. (2021) Inhibited vertical mixing and seasonal persistence of a thin cyanobacterial layer in a stratified lake. Aquatic Sciences 83: 38. doi:10.1007/s00027-021-00785-9
3.       Klindworth A., Pruesse E., Schweer T., et al. (2013) Evaluation of general 16S ribosomal RNA gene PCR primers for classical and next-generation sequencing-based diversity studies. Nucleic Acids Research 41: e1. doi:10.1093/nar/gks808
4.       Minaudo C., Odermatt D., Bouffard D., et al. (2021) The imprint of primary production on high-frequency profiles of lake optical properties. Environmental Science and Technology 55: 14234-14244. doi:10.1021/acs.est.1c02585
5.       Kirtane A., Deiner K. (2024) Improving the recovery for dissolved eDNA state: a comparative analysis of isopropanol precipitation, magnetic bead extraction and centrifugal dialysis. Environmental DNA 6: e572. doi:10.1002/edn3.572
6.       Newton R.J., Jones S.E., Eiler A., et al. (2011) A guide to the natural history of freshwater lake bacteria. Microbiology and Molecular Biology Reviews 75: 14-49. doi:10.1128/MMBR.00028-10
7.      Papadopoulou S., Linkhorst A., Balmonte J.P., et al. (2025) Diel bacterioplankton community dynamics under contrasting light regimes. Environmental Microbiology Reports 17: e70099. doi:10.1111/1758-2229.70099
8.       Schnell I.B., Bohmann K., Gilbert M.T.P. (2015) Tag jumps illuminated: reducing sequence-to-sample misidentifications in metabarcoding studies. Molecular Ecology Resources 15: 1289-1303. doi:10.1111/1755-0998.12402
9.       Wüest A., Bouffard D., Guillard J., et al. (2021) LéXPLORE: a floating laboratory on Lake Geneva offering unique lake research opportunities. WIREs Water 8: e1544. doi:10.1002/wat2.1544



Link
https://natureum.ch/fr/etudier/portraits-de-scientifiques/nadir-alvarez
Animal experimentation
No
