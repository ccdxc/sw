export interface NaplesCondition {
    isHealthy: boolean;
    condition: NaplesConditionValues;

}

export enum NaplesConditionValues {
    HEALTHY = 'Healthy',
    UNHEALTHY = 'Unhealthy',
    UNKNOWN = 'Unknown',
    EMPTY = '',
}

export enum NodeConditionValues {
    HEALTHY = 'Healthy',
    UNHEALTHY = 'Unhealthy',
    UNKNOWN = 'Unknown'
}
