import { MetricTransform, TransformQuery, TransformDataset, TransformNames } from './types';
import { SelectItem } from 'primeng/api';
import { MetricsMetadata } from '@sdk/metrics/generated/metadata';

interface GroupByTransformConfig {
  selectedField: string;
}

/**
 * Populates the group by field tag in metric query and
 * transforms the dataset label to include node name
 */
export class GroupByTransform extends MetricTransform<GroupByTransformConfig> {
  transformName = TransformNames.GroupBy;
  _groupBy: string;
  groupByOptions: SelectItem[] = [];

  onFieldChange() {
    this.updateOptions();
  }

  get groupBy() {
    return this._groupBy;
  }

  set groupBy(value) {
    this._groupBy = value;
    this.requestMetrics();
  }

  updateOptions() {
    if (this.measurement == null) {
      this.groupByOptions = [];
      return;
    }
    const options = MetricsMetadata[this.measurement].fields.filter(x => x.isTag).map( f => {
      return {label: f.displayName, value: f.name};
    });
    options.unshift({label: 'None', value: null});
    this.groupByOptions = options;
  }

  transformQuery(opts: TransformQuery) {
    opts.query['group-by-field'] = this.groupBy;
  }

  transformDataset(opts: TransformDataset) {
    if (opts.series.tags != null) {
      const groupByValue = opts.series.tags[this.groupBy];
      if (groupByValue != null && groupByValue.length > 0) {
        opts.dataset.label += ' - ' + groupByValue;
      }
    } else {
      const tags = MetricsMetadata[opts.measurement].tags;
      if (tags != null && tags.includes('SingleReporter')) {
        // Don't put average if the stat is only reported by one entity
        return;
      }

      opts.dataset.label = 'Avg ' + opts.dataset.label;
    }
  }

  load(config: GroupByTransformConfig) {
    if (config != null && config.selectedField != null && config.selectedField.length > 0) {
      this._groupBy = config.selectedField;
    }
  }

  save(): GroupByTransformConfig {
    return {
      selectedField: this._groupBy,
    };
  }

}
