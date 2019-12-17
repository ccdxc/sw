export interface NaplesCondition {
    isHealthy: boolean;
    condition: string;
    rebootNeeded: boolean;
}

export enum NaplesConditionValues {
    HEALTHY = 'healthy',
    UNHEALTHY = 'unhealthy',
    UNKNOWN = 'unknown',
    EMPTY = '',
    NOTADMITTED = 'not_admitted',
    REBOOT_NEEDED = 'reboot_needed',
}

export enum NodeConditionValues {
    HEALTHY = 'Healthy',
    UNHEALTHY = 'Unhealthy',
    UNKNOWN = 'Unknown'
}
