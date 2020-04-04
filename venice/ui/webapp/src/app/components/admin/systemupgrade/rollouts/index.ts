import { SelectItem } from 'primeng/primeng';


export interface RolloutImageLabel {
    Description: string;
    Environment?: string;
    Releasedate?: string;
    Version: string;
}

export enum EnumRolloutOptions {
    'naplesonly' = 'DSC Only',
    'veniceonly' = 'PSM Only',
    'both' = 'Both DSC and PSM'
}

export interface RolloutImageOption extends SelectItem {
    model?: RolloutImageLabel;
}
