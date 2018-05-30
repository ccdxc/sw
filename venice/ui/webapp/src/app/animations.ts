import {
  trigger,
  state,
  style,
  animate,
  transition
} from '@angular/animations';

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
];
