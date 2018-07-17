import {
  trigger,
  state,
  style,
  animate,
  transition
} from '@angular/animations';

/* Taken from Angular Material expansion panel */
export const EXPANSION_PANEL_ANIMATION_TIMING = '500ms cubic-bezier(0.4,0.0,0.2,1)';

export const Animations = [
  // spinner fading and content entering
  trigger('fade', [
    transition(':enter', [
      style({ opacity: 0 }),
      animate('200ms ease-in')
    ]),
    transition(':leave', [
      animate('200ms ease-in', style({ opacity: 0 }))
    ])
  ]),
  // opacity of the widget when the spinner is there
  trigger('loaded', [
    state('false', style({
      opacity: 0.4
    })),
    state('true', style({
      opacity: 1
    })),
    transition('inactive => active', animate('100ms ease-in')),
    transition('active => inactive', animate('100ms ease-out'))
  ]),
  // For sliding creation panels in and out
  trigger('slideInOut', [
    transition(':enter', [
      style({ height: '0px', visibility: 'hidden', overflow: 'hidden' }),
      animate(EXPANSION_PANEL_ANIMATION_TIMING, style({ height: '*', visibility: 'visible' })),
    ]),
    transition(':leave', [
      style({ height: '*', visibility: 'visible', overflow: 'hidden' }),
      animate(EXPANSION_PANEL_ANIMATION_TIMING, style({ height: '0px', visibility: 'hidden' })),
    ]),
  ]),
];
