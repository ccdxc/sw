import { SelectItem } from 'primeng/primeng';


export interface RolloutImageLabel {
    Description: string;
    Environment?: string;
    Releasedate?: string;
    Version: string;
}

export enum EnumRolloutOptions {
    'naplesonly' = 'NAPLES Only',
    'veniceonly' = 'Venice Only',
    'both' = 'Both NAPLES and Venice'
}

export interface RolloutImageOption extends SelectItem {
    model?: RolloutImageLabel;
}
