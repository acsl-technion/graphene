/*
 * SyncUtils.hpp
 *
 *  Created on: Jun 23, 2016
 *      Author: user
 */

#ifndef ENCLAVE_FORK_TRUSTED_SYNCUTILS_HPP_
#define ENCLAVE_FORK_TRUSTED_SYNCUTILS_HPP_

void spin_lock(int volatile *p);
void spin_unlock(int volatile *p);
void spin_lock_c(unsigned char volatile *p);
void spin_unlock_c(unsigned char volatile *p);

#endif /* ENCLAVE_FORK_TRUSTED_SYNCUTILS_HPP_ */
