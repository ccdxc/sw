import { MetricTransform, TransformDataset, TransformNames } from './types';

/**
 * Modifies the dataset so that they use the displayName from the
 * field metadata instead of name for the label.
 */
export class DisplayLabelTransform extends MetricTransform<{}> {
  transformName = TransformNames.DisplayLabelTransform;

  transformDataset(opts: TransformDataset) {
    opts.dataset.label = this.getFieldData(opts.measurement, opts.field).displayName;
  }

  load(config: {}) {
  }

  save(): {} {
    return {};
  }
}

