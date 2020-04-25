import {
  trigger,
  state,
  style,
  animate,
  transition
} from '@angular/animations';

/* Taken from Angular Material expansion panel */
export const EXPANSION_PANEL_ANIMATION_TIMING = '500ms cubic-bezier(0.4,0.0,0.2,1)';
export const EXPANSION_PANEL_ANIMATION_TIMING_FAST = '250ms cubic-bezier(0.4,0.0,0.2,1)';
export const SIDENAV_WIIDTH = '220px';
export const SIDENAV_WIIDTH_PARTIAL = '80px';

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
      style({
        height: '0px',
        'margin-top': '0px',
        'margin-bottom': '0px',
        'padding-top': '0px',
        'padding-bottom': '0px',
        visibility: 'hidden',
        overflow: 'hidden'
      }),
      animate(EXPANSION_PANEL_ANIMATION_TIMING, style({
          height: '*',
          'margin-top': '*',
          'margin-bottom': '*',
          'padding-top': '*',
          'padding-bottom': '*',
          visibility: 'visible'
        })),
    ]),
    transition(':leave', [
      style({
          height: '*',
          'margin-top': '*',
          'margin-bottom': '*',
          'padding-top': '*',
          'padding-bottom': '*',
          visibility: 'visible',
          overflow: 'hidden'
         }),
      animate(EXPANSION_PANEL_ANIMATION_TIMING, style({
        height: '0px',
        'margin-top': '0px',
        'margin-bottom': '0px',
        'padding-top': '0px',
        'padding-bottom': '0px',
        visibility: 'hidden'
      })),
    ]),
  ]),
  trigger('fastSlideInOut', [
    transition(':enter', [
      style({
        height: '0px',
        'margin-top': '0px',
        'margin-bottom': '0px',
        visibility: 'hidden',
        overflow: 'hidden'
      }),
      animate(EXPANSION_PANEL_ANIMATION_TIMING_FAST, style({
        height: '*',
        'margin-top': '*',
        'margin-bottom': '*',
        'padding-top': '*',
        'padding-bottom': '*',
        visibility: 'visible'
      })),
    ]),
    transition(':leave', [
      style({
        height: '*',
        'margin-top': '*',
        'margin-bottom': '*',
        visibility: 'visible',
        overflow: 'hidden'
       }),
      animate(EXPANSION_PANEL_ANIMATION_TIMING_FAST, style({
        height: '0px',
        'margin-top': '0px',
        'margin-bottom': '0px',
        'padding-top': '0px',
        'padding-bottom': '0px',
        visibility: 'hidden'
      })),
    ]),
  ]),
  trigger('slideInOutLeave', [
    transition(':leave', [
      style({
          height: '*',
          'margin-top': '*',
          'margin-bottom': '*',
          'padding-top': '*',
          'padding-bottom': '*',
          visibility: 'visible',
          overflow: 'hidden'
         }),
      animate(EXPANSION_PANEL_ANIMATION_TIMING, style({
        height: '0px',
        'margin-top': '0px',
        'margin-bottom': '0px',
        'padding-top': '0px',
        'padding-bottom': '0px',
        visibility: 'hidden'
      })),
    ]),
  ]),
  trigger('slideInOutHorizontal', [
    transition(':enter', [
      style({ width: '0px', visibility: 'hidden', overflow: 'hidden' }),
      animate(EXPANSION_PANEL_ANIMATION_TIMING, style({ width: '*', visibility: 'visible' })),
    ]),
    transition(':leave', [
      style({ width: '*', visibility: 'visible', overflow: 'hidden' }),
      animate(EXPANSION_PANEL_ANIMATION_TIMING, style({ width: '0px', visibility: 'hidden' })),
    ]),
  ]),
  trigger('fastSlideInOutHorizontal', [
    transition(':enter', [
      style({ width: '0px', visibility: 'hidden', overflow: 'hidden' }),
      animate(EXPANSION_PANEL_ANIMATION_TIMING_FAST, style({ width: '*', visibility: 'visible' })),
    ]),
    transition(':leave', [
      style({ width: '*', visibility: 'visible', overflow: 'hidden' }),
      animate(EXPANSION_PANEL_ANIMATION_TIMING_FAST, style({ width: '0px', visibility: 'hidden' })),
    ]),
  ]),
  trigger('openCloseFullMainContent', [
    state('close',
      style({
        'margin-left': '0px'
      })
    ),
    state('open',
      style({
        'margin-left': SIDENAV_WIIDTH
      })
    ),
    transition('close => open', animate('250ms ease-in')),
    transition('open => close', animate('250ms ease-in')),
  ]),
  trigger('openClosePartialMainContent', [
    state('close',
      style({
        'margin-left': 0
      })
    ),
    state('open',
      style({
        'margin-left': SIDENAV_WIIDTH_PARTIAL
      })
    ),
    transition('close => open', animate('250ms ease-in')),
    transition('open => close', animate('250ms ease-in')),
  ]),
  trigger('openCloseMainContent', [
    state('close',
      style({
        'margin-left': SIDENAV_WIIDTH_PARTIAL
      })
    ),
    state('open',
      style({
        'margin-left': SIDENAV_WIIDTH
      })
    ),
    transition('close => open', animate('250ms ease-in')),
    transition('open => close', animate('250ms ease-in')),
  ]),
  trigger('openCloseSideNav', [
    state('close',
      style({
        'max-width': SIDENAV_WIIDTH_PARTIAL
      })
    ),
    state('open',
      style({
        'max-width': SIDENAV_WIIDTH
      })
    ),
    transition('close => open', animate('250ms ease-in')),
    transition('open => close', animate('250ms ease-in')),
  ]),
];
