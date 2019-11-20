/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { MonitoringMirrorCollector_type,  MonitoringMirrorCollector_type_uihint  } from './enums';
import { MonitoringMirrorExportConfig, IMonitoringMirrorExportConfig } from './monitoring-mirror-export-config.model';

export interface IMonitoringMirrorCollector {
    'type': MonitoringMirrorCollector_type;
    'export-config'?: IMonitoringMirrorExportConfig;
}


export class MonitoringMirrorCollector extends BaseModel implements IMonitoringMirrorCollector {
    /** Type of Collector */
    'type': MonitoringMirrorCollector_type = null;
    /** When collector type is Venice, collector export information is not required */
    'export-config': MonitoringMirrorExportConfig = null;
    public static propInfo: { [prop in keyof IMonitoringMirrorCollector]: PropInfoItem } = {
        'type': {
            enum: MonitoringMirrorCollector_type_uihint,
            default: 'erspan',
            description:  'Type of Collector',
            required: true,
            type: 'string'
        },
        'export-config': {
            description:  'When collector type is Venice, collector export information is not required',
            required: false,
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringMirrorCollector.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringMirrorCollector.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringMirrorCollector.propInfo[prop] != null &&
                        MonitoringMirrorCollector.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['export-config'] = new MonitoringMirrorExportConfig();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['type'] != null) {
            this['type'] = values['type'];
        } else if (fillDefaults && MonitoringMirrorCollector.hasDefaultValue('type')) {
            this['type'] = <MonitoringMirrorCollector_type>  MonitoringMirrorCollector.propInfo['type'].default;
        } else {
            this['type'] = null
        }
        if (values) {
            this['export-config'].setValues(values['export-config'], fillDefaults);
        } else {
            this['export-config'].setValues(null, fillDefaults);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'type': CustomFormControl(new FormControl(this['type'], [required, enumValidator(MonitoringMirrorCollector_type), ]), MonitoringMirrorCollector.propInfo['type']),
                'export-config': CustomFormGroup(this['export-config'].$formGroup, MonitoringMirrorCollector.propInfo['export-config'].required),
            });
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('export-config') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('export-config').get(field);
                control.updateValueAndValidity();
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['type'].setValue(this['type']);
            this['export-config'].setFormGroupValuesToBeModelValues();
        }
    }
}

