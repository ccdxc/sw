import { ClusterDSCProfile, ClusterDSCProfileSpec, IClusterDSCProfile, IClusterFeature } from '@sdk/v1/models/generated/cluster';


export interface DSCProfileUIModel {
    deploymentTarget: string;
    featureSet: string;
}

export class DSCProfileUtil {
    // static variables
    public static DTARGET_HOST = 'Host';
    public static DTARGET_VIRTUALIZED = 'Virtualized';

    public static FSET_OPT_INTERVMSERVICES = 'InterVMServices';
    public static FSET_OPT_FLOWAWARE = 'FlowAware';
    public static FSET_OPT_FIREWALL = 'Firewall';

    public static FSET_SMARTNIC = 'SmartNIC';
    public static FSET_FLOWAWARE = 'Flow Aware';
    public static FSET_FLOWAWARE_W_FIREWALL = 'Flow Aware with Firewall';

    public static convertUIModel (dscProfile: ClusterDSCProfile): DSCProfileUIModel {

        const dscProfileUI: DSCProfileUIModel = {
            deploymentTarget: '',
            featureSet: ''
        };

        const ivms = dscProfile.spec['feature-set'][DSCProfileUtil.FSET_OPT_INTERVMSERVICES] ? DSCProfileUtil.FSET_OPT_INTERVMSERVICES : null;
        const fa = dscProfile.spec['feature-set'][DSCProfileUtil.FSET_OPT_FLOWAWARE] ? DSCProfileUtil.FSET_OPT_FLOWAWARE : null;
        const fw = dscProfile.spec['feature-set'][DSCProfileUtil.FSET_OPT_FIREWALL] ? DSCProfileUtil.FSET_OPT_FIREWALL : null;

        if (!ivms && !fa && !fw) {
            dscProfileUI.deploymentTarget = DSCProfileUtil.DTARGET_HOST;
            dscProfileUI.featureSet = DSCProfileUtil.FSET_SMARTNIC;
        } else if (!ivms && fa && !fw) {
            dscProfileUI.deploymentTarget = DSCProfileUtil.DTARGET_HOST;
            dscProfileUI.featureSet = DSCProfileUtil.FSET_FLOWAWARE;
        } else if (ivms && fa && fw) {
            dscProfileUI.deploymentTarget = DSCProfileUtil.DTARGET_VIRTUALIZED;
            dscProfileUI.featureSet = DSCProfileUtil.FSET_FLOWAWARE_W_FIREWALL;
        }
        return dscProfileUI;
    }
}
