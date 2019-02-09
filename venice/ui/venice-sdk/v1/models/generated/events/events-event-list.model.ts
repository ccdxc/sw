/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { EventsEvent, IEventsEvent } from './events-event.model';

export interface IEventsEventList {
    'kind'?: string;
    'api-version'?: string;
    'resource-version'?: string;
    'items'?: Array<IEventsEvent>;
}


export class EventsEventList extends BaseModel implements IEventsEventList {
    'kind': string = null;
    'api-version': string = null;
    'resource-version': string = null;
    'items': Array<EventsEvent> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'kind': {
            type: 'string'
        },
        'api-version': {
            type: 'string'
        },
        'resource-version': {
            type: 'string'
        },
        'items': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return EventsEventList.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return EventsEventList.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (EventsEventList.propInfo[prop] != null &&
                        EventsEventList.propInfo[prop].default != null &&
                        EventsEventList.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['items'] = new Array<EventsEvent>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['kind'] != null) {
            this['kind'] = values['kind'];
        } else if (fillDefaults && EventsEventList.hasDefaultValue('kind')) {
            this['kind'] = EventsEventList.propInfo['kind'].default;
        } else {
            this['kind'] = null
        }
        if (values && values['api-version'] != null) {
            this['api-version'] = values['api-version'];
        } else if (fillDefaults && EventsEventList.hasDefaultValue('api-version')) {
            this['api-version'] = EventsEventList.propInfo['api-version'].default;
        } else {
            this['api-version'] = null
        }
        if (values && values['resource-version'] != null) {
            this['resource-version'] = values['resource-version'];
        } else if (fillDefaults && EventsEventList.hasDefaultValue('resource-version')) {
            this['resource-version'] = EventsEventList.propInfo['resource-version'].default;
        } else {
            this['resource-version'] = null
        }
        if (values) {
            this.fillModelArray<EventsEvent>(this, 'items', values['items'], EventsEvent);
        } else {
            this['items'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'kind': CustomFormControl(new FormControl(this['kind']), EventsEventList.propInfo['kind'].description),
                'api-version': CustomFormControl(new FormControl(this['api-version']), EventsEventList.propInfo['api-version'].description),
                'resource-version': CustomFormControl(new FormControl(this['resource-version']), EventsEventList.propInfo['resource-version'].description),
                'items': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<EventsEvent>('items', this['items'], EventsEvent);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['kind'].setValue(this['kind']);
            this._formGroup.controls['api-version'].setValue(this['api-version']);
            this._formGroup.controls['resource-version'].setValue(this['resource-version']);
            this.fillModelArray<EventsEvent>(this, 'items', this['items'], EventsEvent);
        }
    }
}

