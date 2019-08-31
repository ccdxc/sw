import { SelectItem } from 'primeng/primeng';


export interface RolloutImageLabel {
    Description: string;
    Environment?: string;
    Releasedate?: string;
    Version: string;
}

export enum EnumRolloutOptions {
    'naplesonly' = 'DS-Card Only',
    'veniceonly' = 'Venice Only',
    'both' = 'Both DS-Card and Venice'
}

export interface RolloutImageOption extends SelectItem {
    model?: RolloutImageLabel;
}
