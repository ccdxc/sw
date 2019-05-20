import { MetricTransform, TransformDataset } from './types';

/**
 * Modifies the dataset so that they use the displayName from the
 * field metadata instead of name for the label.
 */
export class DisplayLabelTransform extends MetricTransform {
  transformName = 'DisplayLabel';

  transformDataset(opts: TransformDataset) {
    opts.dataset.label = this.getFieldData(opts.measurement, opts.field).displayName;
  }
}

