/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef _IONIC_BUS_H_
#define _IONIC_BUS_H_

int ionic_bus_get_irq(struct ionic *ionic, unsigned int num);
const char *ionic_bus_info(struct ionic *ionic);
int ionic_bus_alloc_irq_vectors(struct ionic *ionic, unsigned int nintrs);
void ionic_bus_free_irq_vectors(struct ionic *ionic);
int ionic_bus_register_driver(void);
void ionic_bus_unregister_driver(void);

#endif /* _IONIC_BUS_H_ */
