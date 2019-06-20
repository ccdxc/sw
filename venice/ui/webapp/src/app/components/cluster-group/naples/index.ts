export interface NaplesCondition {
    isHealthy: boolean;
    condition: NaplesConditionValues;

}

export enum NaplesConditionValues {
    HEALTHY = 'Healthy',
    UNHEALTHY = 'Unhealthy',
    UNKNOWN = 'Unknown'
}
