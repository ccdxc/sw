

export interface DSCProfileUIModel {
    deploymentTarget: string;
    featureSet: string;
}

export class DSCProfileUtil {
    // static variables
    public static DTARGET_HOST = 'host';
    public static DTARGET_VIRTUALIZED = 'virtualized';

    public static FSET_SMARTNIC = 'smartnic';
    public static FSET_FLOWAWARE = 'flowaware';
    public static FSET_FLOWAWARE_W_FIREWALL = 'flowaware_firewall';
}
